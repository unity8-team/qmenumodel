#include "qdbusactiongroup.h"

#include <QDebug>

#include <gio/gio.h>

QDBusActionGroup::QDBusActionGroup(QObject *parent)
    :QObject(parent),
      m_actionGroup(NULL)
{
}

QDBusActionGroup::~QDBusActionGroup()
{
    clear();
}

QAction *QDBusActionGroup::getAction(const QString &actionName)
{
    Q_FOREACH(QAction *act, m_actions) {
        if (act->text() == actionName) {
            return act;
        }
    }

    return NULL;
}

void QDBusActionGroup::serviceVanish(GDBusConnection *)
{
    clear();
}

void QDBusActionGroup::serviceAppear(GDBusConnection *connection)
{
    GDBusActionGroup *ag = g_dbus_action_group_get(connection,
                                                   busName().toLatin1(),
                                                   objectPath().toLatin1());
    setActionGroup(ag);
    if (ag == NULL) {
        stop();
    }
}

void QDBusActionGroup::start()
{
    QDBusObject::connect();
}

void QDBusActionGroup::stop()
{
    QDBusObject::disconnect();
}

void QDBusActionGroup::busTypeChanged(BusType)
{
    busTypeChanged();
}

void QDBusActionGroup::busNameChanged(const QString &)
{
    busNameChanged();
}

void QDBusActionGroup::objectPathChanged(const QString &objectPath)
{
    objectPathChanged();
}

void QDBusActionGroup::statusChanged(ConnectionStatus status)
{
    statusChanged();
}

void QDBusActionGroup::setIntBusType(int busType)
{
    if ((busType > None) && (busType < LastBusType)) {
        setBusType(static_cast<BusType>(busType));
    }
}

void QDBusActionGroup::setActionGroup(GDBusActionGroup *ag)
{
    if (m_actionGroup == reinterpret_cast<GActionGroup*>(ag)) {
        return;
    }

    if (m_actionGroup) {
        g_signal_handler_disconnect(m_actionGroup, m_signalActionAddId);
        g_signal_handler_disconnect(m_actionGroup, m_signalActionRemovedId);
        m_signalActionAddId = m_signalActionRemovedId = 0;
        g_object_unref(m_actionGroup);
    }

    m_actionGroup = reinterpret_cast<GActionGroup*>(ag);

    if (m_actionGroup) {

        m_signalActionAddId = g_signal_connect(m_actionGroup,
                                               "action-add",
                                               G_CALLBACK(QDBusActionGroup::onActionAdded),
                                               this);

        m_signalActionRemovedId = g_signal_connect(m_actionGroup,
                                                   "action-removed",
                                                   G_CALLBACK(QDBusActionGroup::onActionRemoved),
                                                   this);

        gchar **actionNames = g_action_group_list_actions(m_actionGroup);
        for(int i=0; actionNames[i] != NULL; i++) {
            addAction(actionNames[i]);
        }
        g_strfreev(actionNames);
    }
}

void QDBusActionGroup::addAction(const char *actionName)
{
    QAction *act = new QAction(actionName, this);

    act->setEnabled(g_action_group_get_action_enabled(m_actionGroup, actionName));

    const GVariantType *stateType = g_action_group_get_action_state_type(m_actionGroup, actionName);
    if (stateType == G_VARIANT_TYPE_BOOLEAN) {
        act->setCheckable(true);

        GVariant *actState = g_action_group_get_action_state(m_actionGroup, actionName);
        if (actState != NULL) {
            act->setChecked(g_variant_get_boolean(actState));
            g_variant_unref(actState);
        }
    }

    // remove any older action with the same name
    removeAction(actionName);

    m_actions.insert(act);
}

void QDBusActionGroup::removeAction(const char *actionName)
{
    Q_FOREACH(QAction *act, m_actions) {
        if (act->text() == actionName) {
            m_actions.remove(act);
            delete act;
            break;
        }
    }
}

void QDBusActionGroup::updateAction(const char *actionName, GVariant *state)
{
    QAction *action = getAction(actionName);
    if ((action != NULL) && (state != NULL)) {

        const GVariantType *stateType = g_variant_get_type(state);
        if (stateType == G_VARIANT_TYPE_BOOLEAN) {
            action->setChecked(g_variant_get_boolean(state));
        }
    }
}

void QDBusActionGroup::clear()
{
    Q_FOREACH(QAction *act, m_actions) {
        delete act;
    }
    m_actions.clear();

    if (m_actionGroup != NULL) {
        g_object_unref(m_actionGroup);
        m_actionGroup = NULL;
    }
}

void QDBusActionGroup::onActionAdded(GDBusActionGroup *, gchar *actionName, gpointer data)
{
    QDBusActionGroup *self = reinterpret_cast<QDBusActionGroup*>(data);
    self->addAction(actionName);
}

void QDBusActionGroup::onActionRemoved(GDBusActionGroup *, gchar *actionName, gpointer data)
{
    QDBusActionGroup *self = reinterpret_cast<QDBusActionGroup*>(data);
    self->removeAction(actionName);

}

void QDBusActionGroup::onActionStateChanged(GDBusActionGroup *ag, gchar *actionName, GVariant *value, gpointer data)
{
    QDBusActionGroup *self = reinterpret_cast<QDBusActionGroup*>(data);
    self->updateAction(actionName, value);
}
