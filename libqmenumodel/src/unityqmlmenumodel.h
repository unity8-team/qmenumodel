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
    Q_PROPERTY(QByteArray busName READ busName WRITE setBusName NOTIFY busNameChanged)
    Q_PROPERTY(QByteArray actionObjectPath READ actionObjectPath WRITE setActionObjectPath NOTIFY actionObjectPathChanged)
    Q_PROPERTY(QByteArray menuObjectPath READ menuObjectPath WRITE setMenuObjectPath NOTIFY menuObjectPathChanged)

public:
    UnityQmlMenuModel(QObject *parent = NULL);
    ~UnityQmlMenuModel();

    void classBegin();
    void componentComplete();

    QByteArray busName() const;
    void setBusName(const QByteArray &name);

    QByteArray actionObjectPath() const;
    void setActionObjectPath(const QByteArray &path);

    QByteArray menuObjectPath() const;
    void setMenuObjectPath(const QByteArray &path);

Q_SIGNALS:
    void busNameChanged(const QByteArray &name);
    void actionObjectPathChanged(const QByteArray &path);
    void menuObjectPathChanged(const QByteArray &path);

private:
    struct UnityQmlMenuModelPrivate *priv;
};

#endif
