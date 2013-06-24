/*
 * Copyright 2013 Canonical Ltd.
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
 * Authors: Lars Uebernickel <lars.uebernickel@canonical.com>
 */

#ifndef UNITYQMLENUMODEL_H
#define UNITYQMLENUMODEL_H

#include "unitymenumodel.h"

#include <QQmlParserStatus>

class UnityQmlMenuModel: public UnityMenuModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

public:
    UnityQmlMenuModel(QObject *parent = NULL);
    ~UnityQmlMenuModel();

    void classBegin();
    void componentComplete();

private:
    struct UnityQmlMenuModelPrivate *priv;
};

#endif
