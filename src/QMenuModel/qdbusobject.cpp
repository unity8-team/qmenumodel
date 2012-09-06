#include "qdbusobject.h"

#include <QDebug>

QDbusObject::QDbusObject()
    :m_watchId(0),
     m_busType(None),
     m_status(QDbusObject::Disconnected)
{
}

QDbusObject::~QDbusObject()
{
    if (m_watchId != 0) {
        g_bus_unwatch_name (m_watchId);
        m_watchId = 0;
    }
}

QDbusObject::BusType QDbusObject::busType() const
{
    return m_busType;
}

void QDbusObject::setBusType(QDbusObject::BusType type)
{
    if (m_busType != type) {
        if (m_status != QDbusObject::Disconnected)
            disconnect();
        m_busType = type;
        busTypeChanged(m_busType);
    }
}

QString QDbusObject::busName() const
{
    return m_busName;
}

void QDbusObject::setBusName(const QString &busName)
{
    if (m_busName != busName) {
        if (m_status != QDbusObject::Disconnected)
            disconnect();
        m_busName = busName;
        busNameChanged(m_busName);
    }
}

QString QDbusObject::objectPath() const
{
    return m_objectPath;
}

void QDbusObject::setObjectPath(const QString &objectPath)
{
    if (m_objectPath != objectPath) {
        if (m_status != QDbusObject::Disconnected)
            disconnect();
        m_objectPath = objectPath;
        objectPathChanged(m_objectPath);
    }
}

void QDbusObject::setStatus(QDbusObject::ConnectionStatus status)
{
    if (m_status != status) {
        m_status = status;
        statusChanged(m_status);
    }
}

QDbusObject::ConnectionStatus QDbusObject::status() const
{
    return m_status;
}

void QDbusObject::connect()
{
    if (m_status != QDbusObject::Disconnected) {
        return;
    } else if ((m_busType > None) && !m_objectPath.isEmpty() && !m_busName.isEmpty()) {
        GBusType type = m_busType == SessionBus ? G_BUS_TYPE_SESSION : G_BUS_TYPE_SYSTEM;
        m_watchId = g_bus_watch_name (type,
                                      m_busName.toLatin1(),
                                      G_BUS_NAME_WATCHER_FLAGS_NONE,
                                      QDbusObject::onServiceAppeared,
                                      QDbusObject::onServiceFanished,
                                      this,
                                      NULL);

        setStatus(QDbusObject::Connecting);
    } else {
        qWarning() << "Invalid dbus connection args";
    }
}

void QDbusObject::disconnect()
{
    if (m_status != QDbusObject::Disconnected) {
        g_bus_unwatch_name (m_watchId);
        m_watchId = 0;
        setStatus(QDbusObject::Disconnected);
    }
}

void QDbusObject::onServiceAppeared(GDBusConnection *connection, const gchar *, const gchar *, gpointer data)
{
    QDbusObject *self = reinterpret_cast<QDbusObject*>(data);

    self->setStatus(QDbusObject::Connected);
    self->serviceAppear(connection);
}

void QDbusObject::onServiceFanished(GDBusConnection *connection, const gchar *, gpointer data)
{
    QDbusObject *self = reinterpret_cast<QDbusObject*>(data);

    self->setStatus(QDbusObject::Connecting);
    self->serviceVanish(connection);
}
