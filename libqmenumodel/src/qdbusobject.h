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

#include <QString>

#include "dbus-enums.h"

typedef unsigned int guint;
typedef char gchar;
typedef void* gpointer;
typedef struct _GDBusConnection GDBusConnection;

class QDBusObject
{
public:
    QDBusObject();
    ~QDBusObject();

    DBusEnums::BusType busType() const;
    void setBusType(DBusEnums::BusType type);

    QString busName() const;
    void setBusName(const QString &busName);

    QString objectPath() const;
    void setObjectPath(const QString &busName);

    DBusEnums::ConnectionStatus status() const;

    void connect();
    void disconnect();

protected:
    virtual void serviceAppear(GDBusConnection *connection) = 0;
    virtual void serviceVanish(GDBusConnection *connection) = 0;

    // notify functions
    virtual void busTypeChanged(DBusEnums::BusType type) = 0;
    virtual void busNameChanged(const QString &busNameChanged) = 0;
    virtual void objectPathChanged(const QString &objectPath) = 0;
    virtual void statusChanged(DBusEnums::ConnectionStatus status) = 0;

private:
    guint m_watchId;
    DBusEnums::BusType m_busType;
    QString m_busName;
    QString m_objectPath;
    DBusEnums::ConnectionStatus m_status;

    void setStatus(DBusEnums::ConnectionStatus status);

    // glib slots
    static void onServiceAppeared(GDBusConnection *connection, const gchar *name, const gchar *name_owner, gpointer data);
    static void onServiceVanished(GDBusConnection *connection, const gchar *name, gpointer data);
};

#endif
