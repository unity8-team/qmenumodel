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

    static void menuItemInserted(GtkMenuTrackerItem *item, gint position, gpointer user_data);
    static void menuItemRemoved(gint position, gpointer user_data);
    static void menuItemChanged(GObject *object, GParamSpec *pspec, gpointer user_data);
};

UnityMenuModelPrivate::UnityMenuModelPrivate(UnityMenuModel *model,
                                             const QByteArray &busName,
                                             const QByteArray &actionGroupObjectPath,
                                             const QByteArray &menuObjectPath)
{
    GDBusConnection *connection;
    GDBusActionGroup *actions;
    GDBusMenuModel *menu;

    this->model = model;

    connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);
    actions = g_dbus_action_group_get (connection, busName.constData(), actionGroupObjectPath.constData());
    menu = g_dbus_menu_model_get (connection, busName.constData(), menuObjectPath.constData());

    this->muxer = gtk_action_muxer_new ();
    g_object_set_qdata (G_OBJECT (this->muxer), unity_menu_model_quark (), model);
    gtk_action_muxer_insert (this->muxer, "indicator", G_ACTION_GROUP (actions));

    this->menutracker = gtk_menu_tracker_new (GTK_ACTION_OBSERVABLE (this->muxer),
                                              G_MENU_MODEL (menu), TRUE, "indicator",
                                              menuItemInserted, menuItemRemoved, this);

    this->items = g_sequence_new (NULL);

    g_object_unref (menu);
    g_object_unref (actions);
    g_object_unref (connection);
}

UnityMenuModelPrivate::~UnityMenuModelPrivate()
{
    GSequenceIter *it;

    it = g_sequence_get_begin_iter (this->items);
    while (!g_sequence_iter_is_end (it)) {
        GtkMenuTrackerItem *item = (GtkMenuTrackerItem *) g_sequence_get (it);
        g_signal_handlers_disconnect_by_func (item, (gpointer) menuItemChanged, it);
        g_object_unref (item);
        it = g_sequence_iter_next (it);
    }
    g_sequence_free (this->items);

    g_object_unref (this->muxer);
    gtk_menu_tracker_free (this->menutracker);
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

        default:
            return QVariant();
    }
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
    GtkMenuTrackerItem *item;

    priv->model->beginRemoveRows(QModelIndex(), position, position);

    it = g_sequence_get_iter_at_pos (priv->items, position);
    item = (GtkMenuTrackerItem *) g_sequence_get (it);
    g_signal_handlers_disconnect_by_func (item, (gpointer) menuItemChanged, it);
    g_object_unref (item);
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

    return names;
}
