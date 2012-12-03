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

#ifndef QMENUMODEL_H
#define QMENUMODEL_H

#include <QAbstractListModel>

typedef int gint;
typedef unsigned int guint;
typedef void* gpointer;
typedef struct _GMenuModel GMenuModel;
typedef struct _GObject GObject;

class CacheData;

class QMenuModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum MenuRoles {
        Action = 0,
        Label,
        LinkSection,
        LinkSubMenu,
        Extra
    };

    ~QMenuModel();

    Q_INVOKABLE QVariantMap get(int row) const;

    /* QAbstractItemModel */
    QHash<int, QByteArray> roleNames() const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QModelIndex parent (const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

Q_SIGNALS:
    void countChanged();

protected:
    QMenuModel(GMenuModel *other=0, QObject *parent=0);
    void setMenuModel(GMenuModel *model);
    GMenuModel *menuModel() const;

    // help function for test
    QList<CacheData*> cache() const;

private:
    QList<CacheData*> m_cache;
    GMenuModel *m_menuModel;
    guint m_signalChangedId;

    QVariant getStringAttribute(const QModelIndex &index, const QString &attribute) const;
    QVariant getLink(const QModelIndex &index, const QString &linkName);
    QVariant getExtraProperties(const QModelIndex &index) const;
    QString parseExtraPropertyName(const QString &name) const;
    void clearModel();
    int count() const;

    static void onItemsChanged(GMenuModel *model, gint position, gint removed, gint added, gpointer data);
};

#endif
