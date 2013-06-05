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

class UnityMenuModelPrivate
{
public:
    UnityMenuModelPrivate(UnityMenuModel *model,
                          const QByteArray &busName,
                          const QByteArray &actionGroupObjectPath,
                          const QByteArray &menuObjectPath);

    ~UnityMenuModelPrivate();

    int nrItems();
    QVariant data(int position, int role);

private:
    UnityMenuModel *model;
    GtkActionMuxer *muxer;
    GtkMenuTracker *menutracker;
    GSequence *items;
    QByteArray actionGroupObjectPath;
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

UnityMenuModelPrivate::UnityMenuModelPrivate(UnityMenuModel *model,
                                             const QByteArray &busName,
                                             const QByteArray &actionGroupObjectPath,
                                             const QByteArray &menuObjectPath)
{
    this->model = model;
    this->actionGroupObjectPath = actionGroupObjectPath;
    this->menuObjectPath = menuObjectPath;
    this->menutracker = NULL;

    this->muxer = gtk_action_muxer_new ();
    g_object_set_qdata (G_OBJECT (this->muxer), unity_menu_model_quark (), model);

    this->items = g_sequence_new (NULL);

    g_bus_watch_name (G_BUS_TYPE_SESSION, busName.constData(), G_BUS_NAME_WATCHER_FLAGS_AUTO_START,
                      nameAppeared, nameVanished, this, NULL);
}

UnityMenuModelPrivate::~UnityMenuModelPrivate()
{
    g_sequence_foreach_iter_range (g_sequence_get_begin_iter (this->items), g_sequence_get_end_iter (this->items),
                                   freeMenuItem, NULL);
    g_sequence_free (this->items);

    if (this->menutracker)
        gtk_menu_tracker_free (this->menutracker);

    g_object_unref (this->muxer);
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

void UnityMenuModelPrivate::freeMenuItem (gpointer data, gpointer user_data)
{
    GSequenceIter *it = (GSequenceIter *) data;
    GtkMenuTrackerItem *item;

    item = (GtkMenuTrackerItem *) g_sequence_get (it);
    g_signal_handlers_disconnect_by_func (item, (gpointer) menuItemChanged, it);
    g_object_unref (item);
}

void UnityMenuModelPrivate::nameAppeared(GDBusConnection *connection, const gchar *name, const gchar *owner, gpointer user_data)
{
    UnityMenuModelPrivate *priv = (UnityMenuModelPrivate *)user_data;
    GDBusActionGroup *actions;
    GDBusMenuModel *menu;

    priv->model->beginResetModel();

    actions = g_dbus_action_group_get (connection, owner, priv->actionGroupObjectPath.constData());
    menu = g_dbus_menu_model_get (connection, owner, priv->menuObjectPath.constData());

    gtk_action_muxer_insert (priv->muxer, "indicator", G_ACTION_GROUP (actions));
    priv->menutracker = gtk_menu_tracker_new (GTK_ACTION_OBSERVABLE (priv->muxer),
                                              G_MENU_MODEL (menu), TRUE, "indicator",
                                              menuItemInserted, menuItemRemoved, priv);

    priv->model->endResetModel();

    g_object_unref (menu);
    g_object_unref (actions);
}

void UnityMenuModelPrivate::nameVanished(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
    UnityMenuModelPrivate *priv = (UnityMenuModelPrivate *)user_data;
    GSequenceIter *begin;
    GSequenceIter *end;

    priv->model->beginResetModel();

    begin = g_sequence_get_begin_iter (priv->items);
    end = g_sequence_get_end_iter (priv->items);
    g_sequence_foreach_iter_range (begin, end, freeMenuItem, NULL);
    g_sequence_remove_range (begin, end);

    gtk_action_muxer_remove (priv->muxer, "indicator");
    g_clear_pointer (&priv->menutracker, gtk_menu_tracker_free);

    priv->model->endResetModel();
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
}

UnityMenuModel::UnityMenuModel(const QByteArray &busName,
                               const QByteArray &actionGroupObjectPath,
                               const QByteArray &menuObjectPath,
                               QObject *parent):
    QAbstractListModel(parent),
    priv(NULL)
{
}

void UnityMenuModel::init(const QByteArray &busName, const QByteArray &actionGroupObjectPath, const QByteArray &menuObjectPath)
{
    priv = new UnityMenuModelPrivate (this, busName, actionGroupObjectPath, menuObjectPath);
}

UnityMenuModel::~UnityMenuModel()
{
    delete priv;
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
