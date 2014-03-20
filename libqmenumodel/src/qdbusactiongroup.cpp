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
#include "qmenumodelevents.h"

// Qt
#include <QCoreApplication>

extern "C" {
#include <glib.h>
#include <gio/gio.h>
}

/*!
    \qmltype QDBusActionGroup
    \inherits QDBusObject

    \brief A DBusActionGroup implementation to be used with \l QDBusMenuModel

    \b {This component is under heavy development.}

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
     QDBusObject(this),
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

    \b Note: methods should only be called after the Component has completed.
*/
QStateAction *QDBusActionGroup::action(const QString &name)
{
    QStateAction *act = actionImpl(name);
    if (act == 0) {
        act = new QStateAction(this, name);
    }

    return act;
}

QVariant QDBusActionGroup::actionState(const QString &name)
{
    QVariant result;
    GVariant *state = g_action_group_get_action_state(m_actionGroup, name.toUtf8().data());
    result = Converter::toQVariant(state);
    if (state) {
        g_variant_unref(state);
    }
    return result;
}


bool QDBusActionGroup::hasAction(const QString &name)
{
    if (m_actionGroup) {
        return g_action_group_has_action(m_actionGroup, name.toUtf8().data());
    } else {
        return false;
    }
}

QStateAction *QDBusActionGroup::actionImpl(const QString &name)
{
    Q_FOREACH(QStateAction *act, this->findChildren<QStateAction*>()) {
        if (act->name() == name) {
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
                                                   busName().toUtf8().data(),
                                                   objectPath().toUtf8().data());
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
    if ((busType > DBusEnums::None) && (busType < DBusEnums::LastBusType)) {
        setBusType(static_cast<DBusEnums::BusType>(busType));
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
        for(guint i=0; i < g_strv_length(actions); i++) {
            DBusActionVisiblityEvent dave(actions[i], true);
            QCoreApplication::sendEvent(this, &dave);
        }
        g_strfreev(actions);
    }
}

/*! \internal */
void QDBusActionGroup::clear()
{
    Q_FOREACH(QStateAction *act, this->findChildren<QStateAction*>()) {
        Q_EMIT actionVanish(act->name());
    }

    if (m_actionGroup != NULL) {
        g_object_unref(m_actionGroup);
        m_actionGroup = NULL;
    }
}

/*! \internal */
void QDBusActionGroup::updateActionState(const QString &name, const QVariant &state)
{
    if (m_actionGroup != NULL) {
        GVariant* action_state;
        QByteArray nameBytes = name.toUtf8();

        action_state = g_action_group_get_action_state(m_actionGroup, nameBytes.data());
        if (action_state) {
            g_action_group_change_action_state(m_actionGroup,
                                               nameBytes.data(),
                                               Converter::toGVariantWithSchema(state, g_variant_get_type_string(action_state)));

            g_variant_unref(action_state);
        } else {
            g_action_group_change_action_state(m_actionGroup, nameBytes.data(), Converter::toGVariant(state));
        }
    }
}

void QDBusActionGroup::activateAction(const QString &name, const QVariant &parameter)
{
    if (m_actionGroup != NULL) {
        const GVariantType* parameter_type;
        QByteArray nameBytes = name.toUtf8();

        parameter_type = g_action_group_get_action_parameter_type(m_actionGroup, nameBytes.data());
        if (parameter_type) {
            gchar* parameter_type_string;
            parameter_type_string = g_variant_type_dup_string(parameter_type);

            g_action_group_activate_action(m_actionGroup,
                                           nameBytes.data(),
                                           Converter::toGVariantWithSchema(parameter, parameter_type_string));

            g_free(parameter_type_string);
        } else {
            g_action_group_activate_action(m_actionGroup, nameBytes.data(), Converter::toGVariant(parameter));
        }
    }
}

bool QDBusActionGroup::event(QEvent* e)
{
    if (QDBusObject::event(e)) {
        return true;
    } else if (e->type() == DBusActionVisiblityEvent::eventType) {
        DBusActionVisiblityEvent *dave = static_cast<DBusActionVisiblityEvent*>(e);

        if (dave->visible) {
            Q_EMIT actionAppear(dave->name);
        } else {
            Q_EMIT actionVanish(dave->name);
        }
    } else if (e->type() == DBusActionStateEvent::eventType) {
        DBusActionStateEvent *dase = static_cast<DBusActionStateEvent*>(e);

        Q_EMIT actionStateChanged(dase->name, dase->state);
    }
    return QObject::event(e);
}

/*! \internal */
void QDBusActionGroup::onActionAdded(GDBusActionGroup *, gchar *name, gpointer data)
{
    QDBusActionGroup *self = reinterpret_cast<QDBusActionGroup*>(data);

    DBusActionVisiblityEvent dave(name, true);
    QCoreApplication::sendEvent(self, &dave);
}

/*! \internal */
void QDBusActionGroup::onActionRemoved(GDBusActionGroup *, gchar *name, gpointer data)
{
    QDBusActionGroup *self = reinterpret_cast<QDBusActionGroup*>(data);

    DBusActionVisiblityEvent dave(name, false);
    QCoreApplication::sendEvent(self, &dave);
}

/*! \internal */
void QDBusActionGroup::onActionStateChanged(GDBusActionGroup *, gchar *name, GVariant *value, gpointer data)
{
    QDBusActionGroup *self = reinterpret_cast<QDBusActionGroup*>(data);

    DBusActionStateEvent dase(name, Converter::toQVariant(value));
    QCoreApplication::sendEvent(self, &dase);
}
