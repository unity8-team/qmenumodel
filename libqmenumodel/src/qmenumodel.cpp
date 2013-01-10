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
 *      Olivier Tilloy <olivier.tilloy@canonical.com>
 */

extern "C" {
#include <gio/gio.h>
}

#include "qmenumodel.h"
#include "menunode.h"
#include "converter.h"
#include <QCoreApplication>
#include <QThread>

/*!
    \qmltype QMenuModel
    \brief The QMenuModel class implements the base list model for menus

    \b {This component is under heavy development.}

    This is a abstracted class used by \l QDBusMenuModel.
*/
/*! \internal */
QMenuModel::QMenuModel(GMenuModel *other, QObject *parent)
    : QAbstractItemModel(parent),
      m_root(0)
{
    setMenuModel(other);
}

/*! \internal */
QMenuModel::~QMenuModel()
{
    clearModel();
}

/*! \internal */
void QMenuModel::setMenuModel(GMenuModel *other)
{
    if ((m_root != 0) && (m_root->model() == other)) {
        return;
    }

    beginResetModel();

    clearModel();

    if (other) {
        m_root = new MenuNode("", other, 0, 0, this);
    }

    endResetModel();
}

/*! \internal */
void QMenuModel::clearModel()
{
    if (m_root) {
        delete m_root;
        m_root = NULL;
    }
}

/*! \internal */
QHash<int, QByteArray> QMenuModel::roleNames() const
{
    static QHash<int, QByteArray> roles;
    if (roles.isEmpty()) {
        roles[Action] = "action";
        roles[Label] = "label";
        roles[Extra] = "extra";
        roles[Depth] = "depth";
        roles[hasSection] = "hasSection";
        roles[hasSubMenu] = "hasSubMenu";
    }
    return roles;
}

/*! \internal */
QModelIndex QMenuModel::index(int row, int column, const QModelIndex &parent) const
{
    MenuNode *node = nodeFromIndex(parent);
    if (node == 0) {
        return QModelIndex();
    }

    if (parent.isValid()) {
        MenuNode *child = node->child(parent.row());
        if (child) {
            node = child;
        }
    }
    return createIndex(row, column, node);
}

/*! \internal */
QModelIndex QMenuModel::parent(const QModelIndex &index) const
{
    if (index.isValid() && index.internalPointer()) {
        MenuNode *node = nodeFromIndex(index);
        if (node->parent()) {
            return createIndex(node->position(), 0, node->parent());
        }
    }

    return QModelIndex();
}

/*! \internal */
QVariant QMenuModel::data(const QModelIndex &index, int role) const
{
    QVariant attribute;
    if (!index.isValid()) {
        return attribute;
    }

    MenuNode *node = nodeFromIndex(index);
    int row = node ? node->realPosition(index.row()) : -1;

    if (row >= 0) {
        switch (role) {
        case Action:
            attribute = getStringAttribute(node, row, G_MENU_ATTRIBUTE_ACTION);
            break;
        case Qt::DisplayRole:
        case Label:
            attribute = getStringAttribute(node, row, G_MENU_ATTRIBUTE_LABEL);
            break;
        case Extra:
            attribute = getExtraProperties(node, row);
            break;
        case hasSection:
            attribute = QVariant(hasLink(node, row, G_MENU_LINK_SECTION));
            break;
        case hasSubMenu:
            attribute = QVariant(hasLink(node, row, G_MENU_LINK_SUBMENU));
            break;
        case Depth:
            attribute = QVariant(node->depth());
            break;
        default:
            break;
        }
    }
    return attribute;
}

/*! \internal */
int QMenuModel::rowCount(const QModelIndex &index) const
{
    if (index.isValid()) {
        MenuNode *node = nodeFromIndex(index);
        if (node) {
            MenuNode *child = node->child(index.row());
            if (child) {
                return child->size();
            }
        }
        return 0;
    }
    if (m_root) {
        return m_root->size();
    }
    return 0;
}

/*! \internal */
int QMenuModel::columnCount(const QModelIndex &) const
{
    return 1;
}

/*! \internal */
QVariant QMenuModel::getStringAttribute(MenuNode *node,
                                        int row,
                                        const QString &attribute) const
{
    QVariant result;
    gchar* value = NULL;
    g_menu_model_get_item_attribute(node->model(),
                                    row,
                                    attribute.toUtf8().data(),
                                    "s", &value);
    if (value) {
        result = QVariant(QString::fromUtf8(value));
        g_free(value);
    }
    return result;
}

/*! \internal */
QVariant QMenuModel::getExtraProperties(MenuNode *node, int row) const
{
    GMenuAttributeIter *iter = g_menu_model_iterate_item_attributes(node->model(), row);
    if (iter == NULL) {
        return QVariant();
    }

    QVariantMap extra;
    const gchar *attrName = NULL;
    GVariant *value = NULL;
    while (g_menu_attribute_iter_get_next (iter, &attrName, &value)) {
        if (strncmp("x-", attrName, 2) == 0) {
            extra.insert(parseExtraPropertyName(attrName),
                         Converter::toQVariant(value));
        }
    }

    return extra;
}

/*! \internal */
void QMenuModel::onItemsChanged(MenuNode *node,
                                int position,
                                int removed,
                                int added)
{
    QModelIndex index = indexFromNode(node);
    if (removed > 0) {
        beginRemoveRows(index, position, position + removed - 1);

        node->commitOperation();

        endRemoveRows();
    }

    if (added > 0) {
        beginInsertRows(index, position, position + added - 1);

        node->commitOperation();

        endInsertRows();
    }
}

/*! \internal */
QModelIndex QMenuModel::indexFromNode(MenuNode *node) const
{
    return createIndex(node->position(), 0, node);
}

/*! \internal */
MenuNode *QMenuModel::nodeFromIndex(const QModelIndex &index) const
{
    MenuNode *node = 0;

    if (index.isValid()) {
        node = reinterpret_cast<MenuNode*>(index.internalPointer());
    } else {
        node = m_root;
    }

    return node;
}

/*! \internal */
QString QMenuModel::parseExtraPropertyName(const QString &name) const
{
    QString newName(name);
    if (name.startsWith("x-")) {
        newName = name.mid(2);
    }
    return newName.replace("-", "_");
}

GMenuModel *QMenuModel::menuModel() const
{
    return m_root->model();
}

bool QMenuModel::hasLink(MenuNode *node, int row, const QString &linkType) const
{
    MenuNode *child = node->child(row);
    return (child && (child->linkType() == linkType));
}
