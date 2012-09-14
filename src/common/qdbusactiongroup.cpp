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
        return addAction(name.toLatin1(), true);
    } else {
        return act;
    }
}

QStateAction *QDBusActionGroup::actionImpl(const QString &name)
{
    Q_FOREACH(QStateAction *act, m_actions) {
        if (act->text() == name) {
            return act;
        }
    }
    return 0;
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
            addAction(actionNames[i], true);
        }
        g_strfreev(actionNames);
    }
}

/*! \internal */
QStateAction *QDBusActionGroup::addAction(const char *actionName, bool create)
{
    bool isNew = false;
    QStateAction *act = actionImpl(actionName);
    if (act == 0) {
        if (create) {
            act = new QStateAction(actionName, this);
            isNew = true;
        } else {
            return 0;
        }
    }

    if (g_action_group_has_action(m_actionGroup, actionName)) {
        act->setEnabled(g_action_group_get_action_enabled(m_actionGroup, actionName));

        GVariant *actState = g_action_group_get_action_state(m_actionGroup, actionName);
        if (actState) {
            act->setState(Converter::toGVariant(actState));
        }
        act->setValid(true);
    } else {
        act->setValid(false);
    }

    if (isNew) {
        QObject::connect(act, SIGNAL(triggered()), this, SLOT(onActionTriggered()));
        m_actions.insert(act);
        Q_EMIT countChanged(m_actions.count());
    }
    return act;
}

/*! \internal */
void QDBusActionGroup::onActionTriggered()
{
    QStateAction *act = qobject_cast<QStateAction*>(QObject::sender());
    if (act->isValid()) {
        g_action_group_activate_action(m_actionGroup, act->text().toLatin1(), NULL);
    }
}

/*! \internal */
void QDBusActionGroup::removeAction(const char *actionName, bool erase)
{
    Q_FOREACH(QStateAction *act, m_actions) {
        if (act->text() == actionName) {
            if (erase) {
                m_actions.remove(act);
                delete act;
            } else {
                act->setValid(false);
            }
            break;
        }
    }
}

/*! \internal */
void QDBusActionGroup::updateAction(const char *actionName, GVariant *state)
{
    QStateAction *action = this->action(actionName);
    if ((action != NULL) && (state != NULL)) {
        action->setState(Converter::toGVariant(state));
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
    self->addAction(actionName, true);
}

/*! \internal */
void QDBusActionGroup::onActionRemoved(GDBusActionGroup *, gchar *actionName, gpointer data)
{
    QDBusActionGroup *self = reinterpret_cast<QDBusActionGroup*>(data);
    self->removeAction(actionName, false);
}

/*! \internal */
void QDBusActionGroup::onActionStateChanged(GDBusActionGroup *ag, gchar *actionName, GVariant *value, gpointer data)
{
    QDBusActionGroup *self = reinterpret_cast<QDBusActionGroup*>(data);
    self->updateAction(actionName, value);
}
