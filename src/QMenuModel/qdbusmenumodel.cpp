#include "qdbusmenumodel.h"
#include <QDebug>

QDBusMenuModel::QDBusMenuModel(QObject *parent)
    :QMenuModel(parent),
     m_watchId(0),
     m_busType(None)
{
}

QDBusMenuModel::~QDBusMenuModel()
{
    disconnect();
}

QDBusMenuModel::BusType QDBusMenuModel::busType() const
{
    return m_busType;
}

void QDBusMenuModel::setBusType(QDBusMenuModel::BusType type)
{
    if (m_busType != type) {
        if (isConnected())
            disconnect();
        m_busType = type;
        Q_EMIT busTypeChanged(m_busType);
    }
}

QString QDBusMenuModel::busName() const
{
    return m_busName;
}

void QDBusMenuModel::setBusName(const QString &busName)
{
    if (m_busName != busName) {
        if (isConnected())
            disconnect();
        m_busName = busName;
        Q_EMIT busNameChanged(m_busName);
    }
}

QString QDBusMenuModel::objectPath() const
{
    return m_objectPath;
}

void QDBusMenuModel::setObjectPath(const QString &objectPath)
{
    if (m_objectPath != objectPath) {
        if (isConnected())
            disconnect();
        m_objectPath = objectPath;
        Q_EMIT objectPathChanged(m_objectPath);
    }
}

void QDBusMenuModel::connect()
{
    if (isConnected() || (m_watchId > 0)) {
        return;
    } else if ((m_busType > None) && !m_objectPath.isEmpty() && !m_busName.isEmpty()) {
        qDebug() << "Wait for service";
        GBusType type = m_busType == SessionBus ? G_BUS_TYPE_SESSION : G_BUS_TYPE_SYSTEM;
        m_watchId = g_bus_watch_name (type,
                                      m_busName.toLatin1(),
                                      G_BUS_NAME_WATCHER_FLAGS_NONE,
                                      QDBusMenuModel::onServiceAppeared,
                                      QDBusMenuModel::onServiceFanished,
                                      this,
                                      NULL);
    } else {
        Q_EMIT connectionError("Invalid menu model connection args");
    }
}

void QDBusMenuModel::disconnect()
{
    if (isConnected()) {
        g_bus_unwatch_name (m_watchId);
        m_watchId = 0;

        setMenuModel(NULL);
        Q_EMIT disconnected();
    }
}

bool QDBusMenuModel::isConnected() const
{
    return (m_watchId != 0);
}

void QDBusMenuModel::setIntBusType(int busType)
{
    if ((busType > None) && (busType < LastBusType)) {
        setBusType(static_cast<BusType>(busType));
    }
}

void QDBusMenuModel::onServiceAppeared(GDBusConnection *connection, const gchar *, const gchar *, gpointer data)
{
    qDebug() << "Service appears";
    QDBusMenuModel *self = reinterpret_cast<QDBusMenuModel*>(data);
    GMenuModel *model = reinterpret_cast<GMenuModel*>(g_dbus_menu_model_get(connection,
                                                                            self->m_busName.toLatin1(),
                                                                            self->m_objectPath.toLatin1()));
    self->setMenuModel(model);
    if (model) {
        Q_EMIT self->connected();
    } else {
        Q_EMIT self->connectionError("Fail to retrieve menu model");
        self->disconnect();
    }
}

void QDBusMenuModel::onServiceFanished(GDBusConnection *, const gchar *, gpointer data)
{
    qDebug() << "Service fanished";
    QDBusMenuModel *self = reinterpret_cast<QDBusMenuModel*>(data);
    Q_EMIT self->connectionError("Menu model disapear");
    self->disconnect();
}
