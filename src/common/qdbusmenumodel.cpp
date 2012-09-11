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
