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

#include "plugin.h"
#include "qmenumodel.h"
#include "qdbusmenumodel.h"
#include "qdbusactiongroup.h"

#include <QtDeclarative>


void QMenuModelQmlPlugin::registerTypes(const char *uri)
{
    qmlRegisterUncreatableType<QMenuModel>(uri, 0, 1, "QMenuModel",
                                           "QMenuModel is a interface");
    qmlRegisterType<QDBusMenuModel>(uri, 0, 1, "QDBusMenuModel");
    qmlRegisterType<QDBusActionGroup>(uri, 0, 1, "QDBusActionGroup");
}

Q_EXPORT_PLUGIN2(qmenumodel, QMenuModelQmlPlugin)
