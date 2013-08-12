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
#include "converter.h"
#include "actionstateparser.h"
#include "unitymenuaction.h"
#include "unitymenumodelevents.h"

#include <QIcon>
#include <QQmlComponent>
#include <QCoreApplication>

extern "C" {
  #include "gtk/gtkactionmuxer.h"
  #include "gtk/gtkmenutracker.h"
}

G_DEFINE_QUARK (UNITY_MENU_MODEL, unity_menu_model)
G_DEFINE_QUARK (UNITY_SUBMENU_MODEL, unity_submenu_model)
G_DEFINE_QUARK (UNITY_MENU_ITEM_EXTENDED_ATTRIBUTES, unity_menu_item_extended_attributes)

enum MenuRoles {
    LabelRole  = Qt::DisplayRole + 1,
    SensitiveRole,
    IsSeparatorRole,
    IconRole,
    TypeRole,
    ExtendedAttributesRole,
    ActionRole,
    IsCheckRole,
    IsRadioRole
};

class UnityMenuModelPrivate
{
public:
    UnityMenuModelPrivate(UnityMenuModel *model);
    ~UnityMenuModelPrivate();

    void clearItems(bool resetModel=true);
    void clearName();
    void updateActions();
    void updateMenuModel();
    QVariant itemState(GtkMenuTrackerItem *item);

    UnityMenuModel *model;
    GtkActionMuxer *muxer;
    GtkMenuTracker *menutracker;
    GSequence *items;
    GDBusConnection *connection;
    QByteArray busName;
    QByteArray nameOwner;
    guint nameWatchId;
    QVariantMap actions;
    QByteArray menuObjectPath;
    QHash<QByteArray, int> roles;
    ActionStateParser* actionStateParser;

    static void nameAppeared(GDBusConnection *connection, const gchar *name, const gchar *owner, gpointer user_data);
    static void nameVanished(GDBusConnection *connection, const gchar *name, gpointer user_data);
    static void menuItemInserted(GtkMenuTrackerItem *item, gint position, gpointer user_data);
    static void menuItemRemoved(gint position, gpointer user_data);
    static void menuItemChanged(GObject *object, GParamSpec *pspec, gpointer user_data);
};

class UnityGtkMenuTrackerItemAction : public UnityMenuAction
{
public:
    UnityGtkMenuTrackerItemAction(int index, UnityMenuModelPrivate* priv)
        : UnityMenuAction(priv->model),
          d(priv)
    {
        setModel(priv->model);
        setIndex(index);
    }

    virtual QString name() const {
        GtkMenuTrackerItem* item;

        item = (GtkMenuTrackerItem *) g_sequence_get (g_sequence_get_iter_at_pos (d->items, index()));
        if (!item) return QString();

        return gtk_menu_tracker_item_get_action_name(item);
    }

    virtual QVariant state() const {
        GtkMenuTrackerItem* item;

        item = (GtkMenuTrackerItem *) g_sequence_get (g_sequence_get_iter_at_pos (d->items, index()));
        if (!item) return QVariant();

        return d->itemState(item);
    }

    virtual void activate(const QVariant &parameter)
    {
        GtkMenuTrackerItem* item;
        gchar *action;

        item = (GtkMenuTrackerItem *) g_sequence_get (g_sequence_get_iter_at_pos (d->items, index()));
        if (!item) return;

        gtk_menu_tracker_item_get_attribute (item, "action", "s", &action);
        g_action_group_activate_action (G_ACTION_GROUP (d->muxer), action, Converter::toGVariant(parameter));

        g_free (action);
    }

    virtual void changeState(const QVariant& vvalue)
    {
        GtkMenuTrackerItem* item;

        item = (GtkMenuTrackerItem *) g_sequence_get (g_sequence_get_iter_at_pos (d->items, index()));
        if (!item) return;

        GVariant* data = Converter::toGVariant(vvalue);
        gtk_menu_tracker_item_change_state (item, data);
        g_variant_unref(data);
    }

private:
    UnityMenuModelPrivate* d;
};

void menu_item_free (gpointer data)
{
    GtkMenuTrackerItem *item = (GtkMenuTrackerItem *) data;

    g_signal_handlers_disconnect_by_func (item, (gpointer) UnityMenuModelPrivate::menuItemChanged, NULL);
    g_object_unref (item);
}

UnityMenuModelPrivate::UnityMenuModelPrivate(UnityMenuModel *model)
{
    this->model = model;
    this->menutracker = NULL;
    this->connection = NULL;
    this->nameWatchId = 0;
    this->actionStateParser = new ActionStateParser(model);

    this->muxer = gtk_action_muxer_new ();

    this->items = g_sequence_new (menu_item_free);
}

UnityMenuModelPrivate::~UnityMenuModelPrivate()
{
    this->clearItems(false);

    g_clear_pointer (&this->menutracker, gtk_menu_tracker_free);
    g_clear_object (&this->muxer);
    g_clear_object (&this->connection);

    if (this->nameWatchId)
        g_bus_unwatch_name (this->nameWatchId);
}

void UnityMenuModelPrivate::clearItems(bool resetModel)
{
    UnityMenuModelClearEvent ummce(resetModel);
    QCoreApplication::sendEvent(model, &ummce);
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
    Q_FOREACH (QString prefix, this->actions.keys())
        gtk_action_muxer_remove (this->muxer, prefix.toUtf8());

    if (this->nameOwner.isEmpty())
      return;

    for (QVariantMap::const_iterator it = this->actions.constBegin(); it != this->actions.constEnd(); ++it) {
        GDBusActionGroup *actions;

        actions = g_dbus_action_group_get (this->connection, this->nameOwner, it.value().toByteArray());
        gtk_action_muxer_insert (this->muxer, it.key().toUtf8(), G_ACTION_GROUP (actions));

        g_object_unref (actions);
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
                                                  G_MENU_MODEL (menu), TRUE, NULL,
                                                  menuItemInserted, menuItemRemoved, this);

        g_object_unref (menu);
    }
}

QVariant UnityMenuModelPrivate::itemState(GtkMenuTrackerItem *item)
{
    QVariant result;

    GVariant *state = gtk_menu_tracker_item_get_action_state (item);
    if (state != NULL) {
        if (actionStateParser != NULL) {
            result = actionStateParser->toQVariant(state);
        }
        g_variant_unref (state);
    }

    return result;
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

    UnityMenuModelAddRowEvent ummare(item, position);
    QCoreApplication::sendEvent(priv->model, &ummare);
}

void UnityMenuModelPrivate::menuItemRemoved(gint position, gpointer user_data)
{
    UnityMenuModelPrivate *priv = (UnityMenuModelPrivate *)user_data;

    UnityMenuModelRemoveRowEvent ummrre(position);
    QCoreApplication::sendEvent(priv->model, &ummrre);
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
    model = (UnityMenuModel *) g_object_get_qdata (G_OBJECT (item), unity_menu_model_quark ());
    position = g_sequence_iter_get_position (it);

    UnityMenuModelDataChangeEvent ummdce(position);
    QCoreApplication::sendEvent(model, &ummdce);
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
    priv->busName = name;
}

QVariantMap UnityMenuModel::actions() const
{
    return priv->actions;
}

void UnityMenuModel::setActions(const QVariantMap &actions)
{
    priv->actions = actions;
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

ActionStateParser* UnityMenuModel::actionStateParser() const
{
    return priv->actionStateParser;
}

void UnityMenuModel::setActionStateParser(ActionStateParser* actionStateParser)
{
    if (priv->actionStateParser != actionStateParser) {
        if (priv->actionStateParser && priv->actionStateParser->parent() == this) {
            delete priv->actionStateParser;
        }
        priv->actionStateParser = actionStateParser;
        Q_EMIT actionStateParserChanged(actionStateParser);
    }
}

int UnityMenuModel::rowCount(const QModelIndex &parent) const
{
    return !parent.isValid() ? g_sequence_get_length (priv->items) : 0;
}

int UnityMenuModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

static QString iconUri(GIcon *icon)
{
    QString uri;

    if (G_IS_THEMED_ICON (icon)) {
        const gchar* const* iconNames = g_themed_icon_get_names (G_THEMED_ICON (icon));
        guint index = 0;
        while(iconNames[index] != NULL) {
            if (QIcon::hasThemeIcon(iconNames[index])) {
                uri = QString("image://theme/") + iconNames[index];
                break;
            }
            index++;
        }
    }
    else if (G_IS_FILE_ICON (icon)) {
        GFile *file;

        file = g_file_icon_get_file (G_FILE_ICON (icon));
        if (g_file_is_native (file)) {
            gchar *fileuri;

            fileuri = g_file_get_path (file);
            uri = QString(fileuri);

            g_free (fileuri);
        }
    }
    else if (G_IS_BYTES_ICON (icon)) {
        gsize size;
        gconstpointer data;
        gchar *base64;

        data = g_bytes_get_data (g_bytes_icon_get_bytes (G_BYTES_ICON (icon)), &size);
        base64 = g_base64_encode ((const guchar *) data, size);

        uri = QString("data://");
        uri.append (base64);

        g_free (base64);
    }

    return uri;
}

QVariant UnityMenuModel::data(const QModelIndex &index, int role) const
{
    GtkMenuTrackerItem *item;

    item = (GtkMenuTrackerItem *) g_sequence_get (g_sequence_get_iter_at_pos (priv->items, index.row()));

    switch (role) {
        case LabelRole:
            return gtk_menu_tracker_item_get_label (item);

        case SensitiveRole:
            return gtk_menu_tracker_item_get_sensitive (item) == TRUE ? true : false;

        case IsSeparatorRole:
            return gtk_menu_tracker_item_get_is_separator (item) == TRUE ? true : false;

        case IconRole: {
            GIcon *icon = gtk_menu_tracker_item_get_icon (item);
            if (icon) {
                QString uri = iconUri(icon);
                g_object_unref (icon);
                return uri;
            }
            else
                return QString();
        }

        case TypeRole: {
            gchar *type;
            if (gtk_menu_tracker_item_get_attribute (item, "x-canonical-type", "s", &type)) {
                QVariant v(type);
                g_free (type);
                return v;
            }
            else
                return QVariant();
        }

        case ExtendedAttributesRole: {
            QVariantMap *map = (QVariantMap *) g_object_get_qdata (G_OBJECT (item), unity_menu_item_extended_attributes_quark ());
            return map ? *map : QVariant();
        }

        case ActionRole:
            return QVariant::fromValue(new UnityGtkMenuTrackerItemAction(index.row(), priv));

        case IsCheckRole:
            return gtk_menu_tracker_item_get_role (item) == GTK_MENU_TRACKER_ITEM_ROLE_CHECK;

        case IsRadioRole:
            return gtk_menu_tracker_item_get_role (item) == GTK_MENU_TRACKER_ITEM_ROLE_RADIO;

        default:
            return QVariant();
    }
}

QModelIndex UnityMenuModel::index(int row, int column, const QModelIndex &parent) const
{
    return createIndex(row, column);
}

QModelIndex UnityMenuModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

#include <QtDebug>
QHash<int, QByteArray> UnityMenuModel::roleNames() const
{
    QHash<int, QByteArray> names;

    names[LabelRole] = "label";
    names[SensitiveRole] = "sensitive";
    names[IsSeparatorRole] = "isSeparator";
    names[IconRole] = "icon";
    names[TypeRole] = "type";
    names[ExtendedAttributesRole] = "ext";
    names[ActionRole] = "action";
    names[IsCheckRole] = "isCheck";
    names[IsRadioRole] = "isRadio";

    return names;
}

QObject * UnityMenuModel::submenu(int position, QQmlComponent* actionStateParser)
{
    GSequenceIter *it;
    GtkMenuTrackerItem *item;
    UnityMenuModel *model;

    it = g_sequence_get_iter_at_pos (priv->items, position);
    if (g_sequence_iter_is_end (it))
        return NULL;

    item = (GtkMenuTrackerItem *) g_sequence_get (it);
    if (!gtk_menu_tracker_item_get_has_submenu (item))
        return NULL;

    model = (UnityMenuModel *) g_object_get_qdata (G_OBJECT (item), unity_submenu_model_quark ());
    if (model == NULL) {
        model = new UnityMenuModel(this);

        if (actionStateParser) {
            ActionStateParser* parser = qobject_cast<ActionStateParser*>(actionStateParser->create());
            if (parser) {
                model->setActionStateParser(parser);
            }
        }

        model->priv->menutracker = gtk_menu_tracker_new_for_item_submenu (item,
                                                                          UnityMenuModelPrivate::menuItemInserted,
                                                                          UnityMenuModelPrivate::menuItemRemoved,
                                                                          model->priv);
        g_object_set_qdata (G_OBJECT (item), unity_submenu_model_quark (), model);
    }

    return model;
}

static void freeExtendedAttrs(gpointer data)
{
    QVariantMap *extendedAttrs = (QVariantMap *) data;
    delete extendedAttrs;
}

static QVariant attributeToQVariant(GVariant *value, const QString &type)
{
    QVariant result;

    if (type == "int") {
        if (g_variant_is_of_type (value, G_VARIANT_TYPE_INT32))
            result =  QVariant(g_variant_get_int32(value));
    }
    if (type == "bool") {
        if (g_variant_is_of_type (value, G_VARIANT_TYPE_BOOLEAN))
            result = QVariant(g_variant_get_boolean(value));
    }
    else if (type == "string") {
        if (g_variant_is_of_type (value, G_VARIANT_TYPE_STRING))
            result = QVariant(g_variant_get_string(value, NULL));
    }
    if (type == "double") {
        if (g_variant_is_of_type (value, G_VARIANT_TYPE_DOUBLE))
            result = QVariant(g_variant_get_double(value));
    }
    else if (type == "icon") {
        GIcon *icon = g_icon_deserialize (value);
        if (icon) {
            result = iconUri(icon);
            g_object_unref (icon);
        }
        else {
            result = QVariant("");
        }
    }

    return result;
}

/* convert 'some-key' to 'someKey' or 'SomeKey'. (from dconf-qt) */
static QString qtify_name(const char *name)
{
    bool next_cap = false;
    QString result;

    while (*name) {
        if (*name == '-') {
            next_cap = true;
        } else if (next_cap) {
            result.append(toupper(*name));
            next_cap = false;
        } else {
            result.append(*name);
        }

        name++;
    }

    return result;
}

bool UnityMenuModel::loadExtendedAttributes(int position, const QVariantMap &schema)
{
    GtkMenuTrackerItem *item;
    QVariantMap *extendedAttrs;

    item = (GtkMenuTrackerItem *) g_sequence_get (g_sequence_get_iter_at_pos (priv->items, position));

    extendedAttrs = new QVariantMap;

    for (QVariantMap::const_iterator it = schema.constBegin(); it != schema.constEnd(); ++it) {
        QString name = it.key();
        QString type = it.value().toString();

        GVariant *value = gtk_menu_tracker_item_get_attribute_value (item, name.toUtf8(), NULL);
        if (value == NULL) {
            qWarning("loadExtendedAttributes: menu item does not contain '%s'", it.key().toUtf8().constData());
            continue;
        }

        QVariant qvalue = attributeToQVariant(value, type);
        if (qvalue.isValid())
            extendedAttrs->insert(qtify_name (name.toUtf8()), qvalue);
        else
            qWarning("loadExtendedAttributes: key '%s' is of type '%s' (expected '%s')",
                     name.toUtf8().constData(), g_variant_get_type_string(value), type.constData());

        g_variant_unref (value);
    }

    g_object_set_qdata_full (G_OBJECT (item), unity_menu_item_extended_attributes_quark (),
                             extendedAttrs, freeExtendedAttrs);
}

QVariant UnityMenuModel::get(int row, const QByteArray &role)
{
    if (priv->roles.isEmpty()) {
        QHash<int, QByteArray> names = roleNames();
        Q_FOREACH (int role, names.keys())
            priv->roles.insert(names[role], role);
    }

    return this->data(this->index(row, 0), priv->roles[role]);
}

void UnityMenuModel::activate(int index)
{
    GtkMenuTrackerItem *item;

    item = (GtkMenuTrackerItem *) g_sequence_get (g_sequence_get_iter_at_pos (priv->items, index));
    gtk_menu_tracker_item_activated (item);
}

bool UnityMenuModel::event(QEvent* e)
{
    if (e->type() == UnityMenuModelClearEvent::eventType) {
        UnityMenuModelClearEvent *emmce = static_cast<UnityMenuModelClearEvent*>(e);

        GSequenceIter *begin;
        GSequenceIter *end;

        if (emmce->reset)
            beginResetModel();

        begin = g_sequence_get_begin_iter (priv->items);
        end = g_sequence_get_end_iter (priv->items);
        g_sequence_remove_range (begin, end);

        if (emmce->reset)
            endResetModel();

        return true;
    } else if (e->type() == UnityMenuModelAddRowEvent::eventType) {
        UnityMenuModelAddRowEvent *ummrce = static_cast<UnityMenuModelAddRowEvent*>(e);

        GSequenceIter *it;
        it = g_sequence_get_iter_at_pos (priv->items, ummrce->position);
        if (it) {
            beginInsertRows(QModelIndex(), ummrce->position, ummrce->position);

            it = g_sequence_insert_before (it, g_object_ref (ummrce->item));
            g_object_set_qdata (G_OBJECT (ummrce->item), unity_menu_model_quark (), this);
            g_signal_connect (ummrce->item, "notify", G_CALLBACK (UnityMenuModelPrivate::menuItemChanged), it);

            endInsertRows();
        }
        return true;
    } else if (e->type() == UnityMenuModelRemoveRowEvent::eventType) {
        UnityMenuModelRemoveRowEvent *ummrre = static_cast<UnityMenuModelRemoveRowEvent*>(e);

        GSequenceIter *it;
        it = g_sequence_get_iter_at_pos (priv->items, ummrre->position);
        if (it) {
            beginRemoveRows(QModelIndex(), ummrre->position, ummrre->position);

            g_sequence_remove (it);

            endRemoveRows();
        }
        return true;
    } else if (e->type() == UnityMenuModelDataChangeEvent::eventType) {
        UnityMenuModelDataChangeEvent *ummdce = static_cast<UnityMenuModelDataChangeEvent*>(e);

        Q_EMIT dataChanged(index(ummdce->position, 0), index(ummdce->position, 0));
        return true;
    }
    return QAbstractListModel::event(e);
}
