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
#include "qstateaction.h"
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

    Look for a action with the same name and return a \l QStateAction object.

    \bold Note: methods should only be called after the Component has completed.
*/
QStateAction *QDBusActionGroup::action(const QString &name)
{
    QStateAction *act = actionImpl(name);
    if (act == 0) {
        return new QStateAction(this, name);
    } else {
        return act;
    }
}

bool QDBusActionGroup::hasAction(const QString &actionName)
{
    if (m_actionGroup) {
        return g_action_group_has_action(m_actionGroup, actionName.toLatin1());
    } else {
        return false;
    }
}

QStateAction *QDBusActionGroup::actionImpl(const QString &name)
{
    Q_FOREACH(QStateAction *act, this->findChildren<QStateAction*>()) {
        if (act->text() == name) {
            return act;
        }
    }
    return 0;
}

/*! \internal */
void QDBusActionGroup::serviceVanish(GDBusConnection *)
{
    setActionGroup(NULL);
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
        clear();
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

        gchar **actions = g_action_group_list_actions(m_actionGroup);
        for(int i=0; i < g_strv_length(actions); i++) {
            Q_EMIT actionAppear(actions[i]);
        }
        g_strfreev(actions);
    }
}

/*! \internal */
void QDBusActionGroup::clear()
{
    Q_FOREACH(QStateAction *act, this->findChildren<QStateAction*>()) {
        Q_EMIT actionVanish(act->text());
    }

    if (m_actionGroup != NULL) {
        g_object_unref(m_actionGroup);
        m_actionGroup = NULL;
    }
}

/*! \internal */
void QDBusActionGroup::updateActionState(const QString &actionName, const QVariant &state)
{
    if (m_actionGroup != NULL) {
        g_action_group_activate_action(m_actionGroup, actionName.toLatin1(), Converter::toGVariant(state));
    }
}

/*! \internal */
void QDBusActionGroup::onActionAdded(GDBusActionGroup *, gchar *actionName, gpointer data)
{
    QDBusActionGroup *self = reinterpret_cast<QDBusActionGroup*>(data);
    Q_EMIT self->actionAppear(actionName);
}

/*! \internal */
void QDBusActionGroup::onActionRemoved(GDBusActionGroup *, gchar *actionName, gpointer data)
{
    QDBusActionGroup *self = reinterpret_cast<QDBusActionGroup*>(data);
    Q_EMIT self->actionVanish(actionName);
}

/*! \internal */
void QDBusActionGroup::onActionStateChanged(GDBusActionGroup *, gchar *actionName, GVariant *value, gpointer data)
{
    QDBusActionGroup *self = reinterpret_cast<QDBusActionGroup*>(data);
    Q_EMIT self->actionStateChanged(actionName, Converter::toQVariant(value));
}
