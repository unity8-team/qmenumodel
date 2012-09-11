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

#include "qdbusobject.h"

#include <QDebug>

QDBusObject::QDBusObject()
    :m_watchId(0),
     m_busType(None),
     m_status(QDBusObject::Disconnected)
{
    qDebug() << "DBUS CREATED";
    qRegisterMetaType<QDBusObject::ConnectionStatus>("QDBusObject::ConnectionStatus");
}

QDBusObject::~QDBusObject()
{
    if (m_watchId != 0) {
        g_bus_unwatch_name (m_watchId);
        m_watchId = 0;
    }
}

QDBusObject::BusType QDBusObject::busType() const
{
    return m_busType;
}

void QDBusObject::setBusType(QDBusObject::BusType type)
{
    if (m_busType != type) {
        if (m_status != QDBusObject::Disconnected)
            disconnect();
        m_busType = type;
        busTypeChanged(m_busType);
    }
}

QString QDBusObject::busName() const
{
    return m_busName;
}

void QDBusObject::setBusName(const QString &busName)
{
    if (m_busName != busName) {
        if (m_status != QDBusObject::Disconnected)
            disconnect();
        m_busName = busName;
        busNameChanged(m_busName);
    }
}

QString QDBusObject::objectPath() const
{
    return m_objectPath;
}

void QDBusObject::setObjectPath(const QString &objectPath)
{
    if (m_objectPath != objectPath) {
        if (m_status != QDBusObject::Disconnected)
            disconnect();
        m_objectPath = objectPath;
        objectPathChanged(m_objectPath);
    }
}

void QDBusObject::setStatus(QDBusObject::ConnectionStatus status)
{
    if (m_status != status) {
        m_status = status;
        statusChanged(m_status);
    }
}

QDBusObject::ConnectionStatus QDBusObject::status() const
{
    return m_status;
}

void QDBusObject::connect()
{
    if (m_status != QDBusObject::Disconnected) {
        return;
    } else if ((m_busType > None) && !m_objectPath.isEmpty() && !m_busName.isEmpty()) {
        GBusType type = m_busType == SessionBus ? G_BUS_TYPE_SESSION : G_BUS_TYPE_SYSTEM;
        m_watchId = g_bus_watch_name (type,
                                      m_busName.toLatin1(),
                                      G_BUS_NAME_WATCHER_FLAGS_NONE,
                                      QDBusObject::onServiceAppeared,
                                      QDBusObject::onServiceFanished,
                                      this,
                                      NULL);

        setStatus(QDBusObject::Connecting);
    } else {
        qWarning() << "Invalid dbus connection args";
    }
}

void QDBusObject::disconnect()
{
    if (m_status != QDBusObject::Disconnected) {
        g_bus_unwatch_name (m_watchId);
        m_watchId = 0;
        setStatus(QDBusObject::Disconnected);
    }
}

void QDBusObject::onServiceAppeared(GDBusConnection *connection, const gchar *, const gchar *, gpointer data)
{
    QDBusObject *self = reinterpret_cast<QDBusObject*>(data);
    qDebug() << "service appear";

    self->setStatus(QDBusObject::Connected);
    self->serviceAppear(connection);
}

void QDBusObject::onServiceFanished(GDBusConnection *connection, const gchar *, gpointer data)
{
    QDBusObject *self = reinterpret_cast<QDBusObject*>(data);    
    qDebug() << "service disappear";

    self->setStatus(QDBusObject::Connecting);
    self->serviceVanish(connection);
}
