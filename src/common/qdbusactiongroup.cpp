/*
 * Copyright 2012 Canonical Ltd.
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
 * Authors:
 *      Renato Araujo Oliveira Filho <renato@canonical.com>
 */

#include "qdbusactiongroup.h"
#include "converter.h"

#include <QDebug>

#include <gio/gio.h>

/*!
    \qmlclass QDBusActionGroup
    \inherits QDBusObject

    \brief A DBusActionGroup implementation to be used with \l QDBusMenuModel

    \bold {This component is under heavy development.}

     This class can be used as a proxy for an action group that is exported over D-Bus

    \code
    QDBusActionGroup {
        id: actionGroup
        busType: 1
        busName: "com.ubuntu.menu"
        objectPath: "com/ubuntu/menu/actions"
    }

    Button {
        onClicked: actionGroup.getAction("app.quit").trigger()
    }
    \endcode
*/

/*! \internal */
QDBusActionGroup::QDBusActionGroup(QObject *parent)
    :QObject(parent),
      m_actionGroup(NULL)
{
}

/*! \internal */
QDBusActionGroup::~QDBusActionGroup()
{
    clear();
}

/*!
    \qmlmethod QDBusActionGroup::action(QString name)

    Look for a action with the same name and return a \l QAction object.

    \bold Note: methods should only be called after the Component has completed.
*/
QAction *QDBusActionGroup::action(const QString &name)
{
    Q_FOREACH(QAction *act, m_actions) {
        if (act->text() == name) {
            return act;
        }
    }

    return NULL;
}

/*!
    \qmlproperty int QDBusActionGroup::count
    This property holds the number of actions inside of \l QDBusActionGroup
*/
int QDBusActionGroup::count() const
{
    return m_actions.count();
}

/*! \internal */
void QDBusActionGroup::serviceVanish(GDBusConnection *)
{
    clear();
}

/*! \internal */
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

/*! \internal */
void QDBusActionGroup::start()
{
    QDBusObject::connect();
}

/*! \internal */
void QDBusActionGroup::stop()
{
    QDBusObject::disconnect();
}

/*! \internal */
void QDBusActionGroup::setIntBusType(int busType)
{
    if ((busType > None) && (busType < LastBusType)) {
        setBusType(static_cast<BusType>(busType));
    }
}

/*! \internal */
void QDBusActionGroup::setActionGroup(GDBusActionGroup *ag)
{
    if (m_actionGroup == reinterpret_cast<GActionGroup*>(ag)) {
        return;
    }

    if (m_actionGroup) {
        g_signal_handler_disconnect(m_actionGroup, m_signalActionAddId);
        g_signal_handler_disconnect(m_actionGroup, m_signalActionRemovedId);
        g_signal_handler_disconnect(m_actionGroup, m_signalStateChangedId);
        m_signalActionAddId = m_signalActionRemovedId = m_signalStateChangedId = 0;
        g_object_unref(m_actionGroup);
    }

    m_actionGroup = reinterpret_cast<GActionGroup*>(ag);

    if (m_actionGroup) {
        m_signalActionAddId = g_signal_connect(m_actionGroup,
                                               "action-added",
                                               G_CALLBACK(QDBusActionGroup::onActionAdded),
                                               this);

        m_signalActionRemovedId = g_signal_connect(m_actionGroup,
                                                   "action-removed",
                                                   G_CALLBACK(QDBusActionGroup::onActionRemoved),
                                                   this);

        m_signalStateChangedId = g_signal_connect(m_actionGroup,
                                                  "action-state-changed",
                                                   G_CALLBACK(QDBusActionGroup::onActionStateChanged),
                                                   this);

        gchar **actionNames = g_action_group_list_actions(m_actionGroup);
        for(int i=0; actionNames[i] != NULL; i++) {
            addAction(actionNames[i]);
        }
        g_strfreev(actionNames);
    }
}

/*! \internal */
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

    QObject::connect(act, SIGNAL(triggered()), this, SLOT(onActionTriggered()));

    // remove any older action with the same name
    removeAction(actionName);

    m_actions.insert(act);
    Q_EMIT countChanged(m_actions.count());
}

/*! \internal */
void QDBusActionGroup::onActionTriggered()
{
    QAction *act = qobject_cast<QAction*>(QObject::sender());
    g_action_group_activate_action(m_actionGroup, act->text().toLatin1(), NULL);
}

/*! \internal */
void QDBusActionGroup::removeAction(const char *actionName)
{
    Q_FOREACH(QAction *act, m_actions) {
        if (act->text() == actionName) {
            m_actions.remove(act);
            delete act;
            Q_EMIT countChanged(m_actions.count());
            break;
        }
    }
}

/*! \internal */
void QDBusActionGroup::updateAction(const char *actionName, GVariant *state)
{
    QAction *action = this->action(actionName);
    if ((action != NULL) && (state != NULL)) {

        const GVariantType *stateType = g_variant_get_type(state);
        if (stateType == G_VARIANT_TYPE_BOOLEAN) {
            action->setChecked(g_variant_get_boolean(state));
        }

        Q_EMIT actionStateChanged(actionName, Converter::parseGVariant(state));
    }
}

/*! \internal */
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

/*! \internal */
void QDBusActionGroup::onActionAdded(GDBusActionGroup *, gchar *actionName, gpointer data)
{
    QDBusActionGroup *self = reinterpret_cast<QDBusActionGroup*>(data);
    self->addAction(actionName);
}

/*! \internal */
void QDBusActionGroup::onActionRemoved(GDBusActionGroup *, gchar *actionName, gpointer data)
{
    QDBusActionGroup *self = reinterpret_cast<QDBusActionGroup*>(data);
    self->removeAction(actionName);

}

/*! \internal */
void QDBusActionGroup::onActionStateChanged(GDBusActionGroup *ag, gchar *actionName, GVariant *value, gpointer data)
{
    QDBusActionGroup *self = reinterpret_cast<QDBusActionGroup*>(data);
    self->updateAction(actionName, value);
}
