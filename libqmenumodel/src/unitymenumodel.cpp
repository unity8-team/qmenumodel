/*
 * Copyright 2013 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Lars Uebernickel <lars.uebernickel@canonical.com>
 */

#include "unitymenumodel.h"

extern "C" {
  #include "gtk/gtkactionmuxer.h"
  #include "gtk/gtkmenutracker.h"
}

G_DEFINE_QUARK (UNITY_MENU_MODEL, unity_menu_model)
G_DEFINE_QUARK (UNITY_SUBMENU_MODEL, unity_submenu_model)

class UnityMenuModelPrivate
{
public:
    UnityMenuModelPrivate(UnityMenuModel *model);

    static UnityMenuModelPrivate * forSubMenu(UnityMenuModel *model, GtkMenuTrackerItem *item);

    ~UnityMenuModelPrivate();
    int nrItems();
    QVariant data(int position, int role);
    void activate(int position);
    UnityMenuModel * submenu(int position);

    void clearItems(bool resetModel=true);
    void clearName();
    void updateActions();
    void updateMenuModel();

    UnityMenuModel *model;
    GtkActionMuxer *muxer;
    GtkMenuTracker *menutracker;
    GSequence *items;
    GDBusConnection *connection;
    QByteArray busName;
    QByteArray nameOwner;
    guint nameWatchId;
    QByteArray actionObjectPath;
    QByteArray menuObjectPath;

    static void freeMenuItem(gpointer data, gpointer user_data);
    static void nameAppeared(GDBusConnection *connection, const gchar *name, const gchar *owner, gpointer user_data);
    static void nameVanished(GDBusConnection *connection, const gchar *name, gpointer user_data);
    static void menuItemInserted(GtkMenuTrackerItem *item, gint position, gpointer user_data);
    static void menuItemRemoved(gint position, gpointer user_data);
    static void menuItemChanged(GObject *object, GParamSpec *pspec, gpointer user_data);
};

/*
 * Same as g_sequence_foreach_range, but calls func with the GSequenceIter
 * instead of the item.
 */
static void
g_sequence_foreach_iter_range (GSequenceIter *begin,
                               GSequenceIter *end,
                               GFunc          func,
                               gpointer       user_data)
{
    GSequenceIter *it;

    for (it = begin; it != end; it = g_sequence_iter_next (it))
        func (it, user_data);
}

UnityMenuModelPrivate::UnityMenuModelPrivate(UnityMenuModel *model)
{
    this->model = model;
    this->menutracker = NULL;
    this->nameWatchId = 0;

    this->muxer = gtk_action_muxer_new ();
    g_object_set_qdata (G_OBJECT (this->muxer), unity_menu_model_quark (), model);

    this->items = g_sequence_new (NULL);
}

UnityMenuModelPrivate * UnityMenuModelPrivate::forSubMenu(UnityMenuModel *model, GtkMenuTrackerItem *item)
{
    UnityMenuModelPrivate *priv = new UnityMenuModelPrivate(model);

    priv->menutracker = gtk_menu_tracker_new_for_item_submenu (item, menuItemInserted, menuItemRemoved, priv);

    return priv;
}

UnityMenuModelPrivate::~UnityMenuModelPrivate()
{
    this->clearItems(false);

    if (this->menutracker)
        gtk_menu_tracker_free (this->menutracker);

    if (this->muxer)
        g_object_unref (this->muxer);

    g_clear_object (&this->connection);

    if (this->nameWatchId)
        g_bus_unwatch_name (this->nameWatchId);
}

int UnityMenuModelPrivate::nrItems()
{
    return g_sequence_get_length (this->items);
}

QVariant UnityMenuModelPrivate::data(int position, int role)
{
    GtkMenuTrackerItem *item;

    item = (GtkMenuTrackerItem *) g_sequence_get (g_sequence_get_iter_at_pos (this->items, position));

    switch (role) {
        case UnityMenuModel::LabelRole:
            return gtk_menu_tracker_item_get_label (item);

        case UnityMenuModel::SensitiveRole:
            return gtk_menu_tracker_item_get_sensitive (item);

        case UnityMenuModel::IsSeparatorRole:
            return gtk_menu_tracker_item_get_is_separator (item);

        default:
            return QVariant();
    }
}

void UnityMenuModelPrivate::activate(int position)
{
    GtkMenuTrackerItem *item;

    item = (GtkMenuTrackerItem *) g_sequence_get (g_sequence_get_iter_at_pos (this->items, position));
    gtk_menu_tracker_item_activated (item);
}

UnityMenuModel * UnityMenuModelPrivate::submenu(int position)
{
    GSequenceIter *it;
    GtkMenuTrackerItem *item;
    UnityMenuModel *model;

    it = g_sequence_get_iter_at_pos (this->items, position);
    if (g_sequence_iter_is_end (it))
        return NULL;

    item = (GtkMenuTrackerItem *) g_sequence_get (it);
    if (!gtk_menu_tracker_item_get_has_submenu (item))
        return NULL;

    model = (UnityMenuModel *) g_object_get_qdata (G_OBJECT (item), unity_submenu_model_quark ());
    if (model == NULL) {
        model = new UnityMenuModel(this->model);
        model->priv = UnityMenuModelPrivate::forSubMenu(model, item);
        g_object_set_qdata (G_OBJECT (item), unity_submenu_model_quark (), model);
    }

    return model;
}

void UnityMenuModelPrivate::freeMenuItem (gpointer data, gpointer user_data)
{
    GSequenceIter *it = (GSequenceIter *) data;
    GtkMenuTrackerItem *item;

    item = (GtkMenuTrackerItem *) g_sequence_get (it);
    g_signal_handlers_disconnect_by_func (item, (gpointer) menuItemChanged, it);
    g_object_unref (item);
}

void UnityMenuModelPrivate::clearItems(bool resetModel)
{
    GSequenceIter *begin;
    GSequenceIter *end;

    if (resetModel)
        model->beginResetModel();

    begin = g_sequence_get_begin_iter (this->items);
    end = g_sequence_get_end_iter (this->items);
    g_sequence_foreach_iter_range (begin, end, freeMenuItem, NULL);
    g_sequence_remove_range (begin, end);

    if (resetModel)
        model->endResetModel();
}

void UnityMenuModelPrivate::clearName()
{
    this->clearItems();

    this->nameOwner = QByteArray();

    this->updateActions();
    this->updateMenuModel();
}

void UnityMenuModelPrivate::updateActions()
{
    if (!this->nameOwner.isEmpty()) {
        GDBusActionGroup *actions;

        actions = g_dbus_action_group_get (this->connection, this->nameOwner, this->actionObjectPath.constData());
        gtk_action_muxer_insert (this->muxer, "indicator", G_ACTION_GROUP (actions));

        g_object_unref (actions);
    }
    else {
        gtk_action_muxer_remove (this->muxer, "indicator");
    }
}

void UnityMenuModelPrivate::updateMenuModel()
{
    this->clearItems();
    g_clear_pointer (&this->menutracker, gtk_menu_tracker_free);

    if (!this->nameOwner.isEmpty()) {
        GDBusMenuModel *menu;

        menu = g_dbus_menu_model_get (this->connection, this->nameOwner, this->menuObjectPath.constData());
        this->menutracker = gtk_menu_tracker_new (GTK_ACTION_OBSERVABLE (this->muxer),
                                                  G_MENU_MODEL (menu), TRUE, "indicator",
                                                  menuItemInserted, menuItemRemoved, this);

        g_object_unref (menu);
    }
}

void UnityMenuModelPrivate::nameAppeared(GDBusConnection *connection, const gchar *name, const gchar *owner, gpointer user_data)
{
    UnityMenuModelPrivate *priv = (UnityMenuModelPrivate *)user_data;

    priv->connection = (GDBusConnection *) g_object_ref (connection);
    priv->nameOwner = owner;

    priv->updateActions();
    priv->updateMenuModel();
}

void UnityMenuModelPrivate::nameVanished(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
    UnityMenuModelPrivate *priv = (UnityMenuModelPrivate *)user_data;

    priv->clearName();
}

void UnityMenuModelPrivate::menuItemInserted(GtkMenuTrackerItem *item, gint position, gpointer user_data)
{
    UnityMenuModelPrivate *priv = (UnityMenuModelPrivate *)user_data;
    GSequenceIter *it;

    priv->model->beginInsertRows(QModelIndex(), position, position);

    it = g_sequence_get_iter_at_pos (priv->items, position);
    g_signal_connect (item, "notify", G_CALLBACK (menuItemChanged), it);
    g_sequence_insert_before (it, g_object_ref (item));

    priv->model->endInsertRows();
}

void UnityMenuModelPrivate::menuItemRemoved(gint position, gpointer user_data)
{
    UnityMenuModelPrivate *priv = (UnityMenuModelPrivate *)user_data;
    GSequenceIter *it;

    priv->model->beginRemoveRows(QModelIndex(), position, position);

    it = g_sequence_get_iter_at_pos (priv->items, position);
    freeMenuItem ((gpointer) it, NULL);
    g_sequence_remove (it);

    priv->model->endRemoveRows();
}

void UnityMenuModelPrivate::menuItemChanged(GObject *object, GParamSpec *pspec, gpointer user_data)
{
    GSequenceIter *it = (GSequenceIter *) user_data;
    GtkMenuTrackerItem *item;
    GtkActionObservable *muxer;
    UnityMenuModel *model;
    gint position;

    item = (GtkMenuTrackerItem *) g_sequence_get (it);
    muxer = _gtk_menu_tracker_item_get_observable (item);
    model = (UnityMenuModel *) g_object_get_qdata (G_OBJECT (muxer), unity_menu_model_quark ());
    position = g_sequence_iter_get_position (it);

    Q_EMIT model->dataChanged(model->index(position, 0), model->index(position, 0));
}

UnityMenuModel::UnityMenuModel(QObject *parent):
    QAbstractListModel(parent)
{
    priv = new UnityMenuModelPrivate(this);
}

UnityMenuModel::~UnityMenuModel()
{
    delete priv;
}

QByteArray UnityMenuModel::busName() const
{
    return priv->busName;
}

void UnityMenuModel::setBusName(const QByteArray &name)
{
    priv->clearName();

    if (priv->nameWatchId)
        g_bus_unwatch_name (priv->nameWatchId);

    priv->nameWatchId = g_bus_watch_name (G_BUS_TYPE_SESSION, name.constData(), G_BUS_NAME_WATCHER_FLAGS_AUTO_START,
                                          UnityMenuModelPrivate::nameAppeared, UnityMenuModelPrivate::nameVanished,
                                          priv, NULL);
}

QByteArray UnityMenuModel::actionObjectPath() const
{
    return priv->actionObjectPath;
}

void UnityMenuModel::setActionObjectPath(const QByteArray &path)
{
    priv->actionObjectPath = path;
    priv->updateActions();
}

QByteArray UnityMenuModel::menuObjectPath() const
{
    return priv->menuObjectPath;
}

void UnityMenuModel::setMenuObjectPath(const QByteArray &path)
{
    priv->menuObjectPath = path;
    priv->updateMenuModel();
}

int UnityMenuModel::rowCount(const QModelIndex &parent) const
{
    return priv && !parent.isValid() ? priv->nrItems() : 0;
}

int UnityMenuModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant UnityMenuModel::data(const QModelIndex &index, int role) const
{
    return priv ? priv->data(index.row(), role) : QVariant();
}

QModelIndex UnityMenuModel::index(int row, int column, const QModelIndex &parent) const
{
    return createIndex(row, column);
}

QModelIndex UnityMenuModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

QHash<int, QByteArray> UnityMenuModel::roleNames() const
{
    QHash<int, QByteArray> names;

    names[LabelRole] = "label";
    names[ActionRole] = "action";
    names[SensitiveRole] = "sensitive";
    names[IsSeparatorRole] = "isSeparator";

    return names;
}

QObject * UnityMenuModel::submenu(int position)
{
    return priv ? priv->submenu(position) : NULL;
}

void UnityMenuModel::activate(int index)
{
    if (priv)
        priv->activate(index);
}
