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

#include "unityqmlmenumodel.h"

struct UnityQmlMenuModelPrivate
{
    QByteArray busName;
    QByteArray actionObjectPath;
    QByteArray menuObjectPath;
};

UnityQmlMenuModel::UnityQmlMenuModel(QObject *parent):
    UnityMenuModel(parent)
{
    priv = new UnityQmlMenuModelPrivate;
}

UnityQmlMenuModel::~UnityQmlMenuModel()
{
    delete priv;
}

void UnityQmlMenuModel::classBegin()
{
}

void UnityQmlMenuModel::componentComplete()
{
    if (priv->busName.isEmpty())
        qWarning("UnityQmlMenuModel: property 'busName' must be set");
    else if (priv->actionObjectPath.isEmpty())
        qWarning("UnityQmlMenuModel: property 'actionObjectPath' must be set");
    else if (priv->menuObjectPath.isEmpty())
        qWarning("UnityQmlMenuModel: property 'menuObjectPath' must be set");
    else
        UnityQmlMenuModel::init(priv->busName, priv->actionObjectPath, priv->menuObjectPath);
}

QByteArray UnityQmlMenuModel::busName() const
{
    return priv->busName;
}

void UnityQmlMenuModel::setBusName(const QByteArray &name)
{
    if (!priv->busName.isEmpty()) {
        qWarning("UnityQmlMenuModel: cannot change bus name after creation");
        return;
    }

    priv->busName = name;
    Q_EMIT busNameChanged(name);
}

QByteArray UnityQmlMenuModel::actionObjectPath() const
{
    return priv->actionObjectPath;
}

void UnityQmlMenuModel::setActionObjectPath(const QByteArray &path)
{
    if (!priv->actionObjectPath.isEmpty()) {
        qWarning("UnityQmlMenuModel: cannot change object paths after creation");
        return;
    }

    priv->actionObjectPath = path;
    Q_EMIT actionObjectPathChanged(path);
}

QByteArray UnityQmlMenuModel::menuObjectPath() const
{
    return priv->menuObjectPath;
}

void UnityQmlMenuModel::setMenuObjectPath(const QByteArray &path)
{
    if (!priv->menuObjectPath.isEmpty()) {
        qWarning("UnityQmlMenuModel: cannot change object paths after creation");
        return;
    }

    priv->menuObjectPath = path;
    Q_EMIT menuObjectPathChanged(path);
}

