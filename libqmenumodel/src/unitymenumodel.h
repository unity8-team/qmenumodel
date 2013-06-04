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

#ifndef UNITYMENUMODEL_H
#define UNITYMENUMODEL_H

#include <QAbstractListModel>

class UnityMenuModel: public QAbstractListModel
{
    Q_OBJECT

public:
    enum MenuRoles {
        ActionRole  = Qt::DisplayRole + 1,
        LabelRole,
        SensitiveRole
    };

public:
    UnityMenuModel(const QByteArray &busName, const QByteArray &actionGroupObjectPath,
                   const QByteArray &menuObjectPath, QObject *parent = NULL);
    virtual ~UnityMenuModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    QHash<int, QByteArray> roleNames() const;

protected:
    UnityMenuModel(QObject *parent = NULL);
    void init(const QByteArray &busName, const QByteArray &actionGroupObjectPath, const QByteArray &menuObjectPath);

private:
    class UnityMenuModelPrivate *priv;
    friend class UnityMenuModelPrivate;
};

#endif
