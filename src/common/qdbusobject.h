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

#ifndef QDBUSOBJECT_H
#define QDBUSOBJECT_H

#include <QObject>

#include <gio/gio.h>

class QDBusObject
{
public:
    enum BusType {
        None = 0,
        SessionBus,
        SystemBus,
        LastBusType
    };

    enum ConnectionStatus {
        Disconnected = 0,
        Connecting,
        Connected
    };

    QDBusObject();
    ~QDBusObject();

    BusType busType() const;
    void setBusType(BusType type);

    QString busName() const;
    void setBusName(const QString &busName);

    QString objectPath() const;
    void setObjectPath(const QString &busName);

    ConnectionStatus status() const;

    void connect();
    void disconnect();

protected:
    virtual void serviceAppear(GDBusConnection *connection) = 0;
    virtual void serviceVanish(GDBusConnection *connection) = 0;

    // notify functions
    virtual void busTypeChanged(BusType type) = 0;
    virtual void busNameChanged(const QString &busNameChanged) = 0;
    virtual void objectPathChanged(const QString &objectPath) = 0;
    virtual void statusChanged(ConnectionStatus status) = 0;

private:
    guint m_watchId;
    BusType m_busType;
    QString m_busName;
    QString m_objectPath;
    ConnectionStatus m_status;

    void setStatus(ConnectionStatus status);

    // glib slots
    static void onServiceAppeared(GDBusConnection *connection, const gchar *name, const gchar *name_owner, gpointer data);
    static void onServiceFanished(GDBusConnection *connection, const gchar *name, gpointer data);
};

#endif
