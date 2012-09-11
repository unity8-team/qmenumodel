#include "qdbusmenumodel.h"
#include <QDebug>

QDBusMenuModel::QDBusMenuModel(QObject *parent)
    :QMenuModel(0, parent)
{
}

QDBusMenuModel::~QDBusMenuModel()
{
}

void QDBusMenuModel::start()
{
    QDBusObject::connect();
}

void QDBusMenuModel::stop()
{
    QDBusObject::disconnect();
}

void QDBusMenuModel::serviceVanish(GDBusConnection *)
{
    setMenuModel(NULL);
}

void QDBusMenuModel::serviceAppear(GDBusConnection *connection)
{
    GMenuModel *model = reinterpret_cast<GMenuModel*>(g_dbus_menu_model_get(connection,
                                                                            busName().toLatin1(),
                                                                            objectPath().toLatin1()));
    setMenuModel(model);
    if (model == NULL) {
        stop();
    }
}

/*
void QDBusMenuModel::busTypeChanged(BusType)
{
    busTypeChanged();
}

void QDBusMenuModel::busNameChanged(const QString &)
{
    busNameChanged();
}

void QDBusMenuModel::objectPathChanged(const QString &objectPath)
{
    objectPathChanged();
}

void QDBusMenuModel::statusChanged(ConnectionStatus status)
{
    statusChanged();
}
*/

void QDBusMenuModel::setIntBusType(int busType)
{
    if ((busType > None) && (busType < LastBusType)) {
        setBusType(static_cast<BusType>(busType));
    }
}
