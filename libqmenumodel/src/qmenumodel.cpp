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

#include "qmenumodel.h"
#include "converter.h"

#include <QDebug>

/*!
    \qmltype QMenuModel
    \brief The QMenuModel class implements the base list model for menus

    \b {This component is under heavy development.}

    This is a abstracted class used by \l QDBusMenuModel.
*/

/*! \internal */
QMenuModel::QMenuModel(GMenuModel *other, QObject *parent)
    : QAbstractListModel(parent),
      m_menuModel(0),
      m_signalChangedId(0)
{
    static QHash<int, QByteArray> rolesNames;
    if (rolesNames.empty()) {
        rolesNames[Action] = "action";
        rolesNames[Label] = "label";
        rolesNames[LinkSection] = "linkSection";
        rolesNames[LinkSubMenu] = "linkSubMenu";
        rolesNames[Extra] = "extra";
    }
    setRoleNames(rolesNames);
    setMenuModel(other, true);
}

/*! \internal */
QMenuModel::~QMenuModel()
{
    setMenuModel(NULL, false);
}

/*! \internal */
void QMenuModel::setMenuModel(GMenuModel *other, bool notify)
{
    if (m_menuModel == other) {
        return;
    }

    if (notify) {
        beginResetModel();
    }

    if (m_menuModel) {
        g_signal_handler_disconnect(m_menuModel, m_signalChangedId);
        m_signalChangedId = 0;
        g_object_unref(m_menuModel);
    }

    m_menuModel = other;

    if (m_menuModel) {
        // this will trigger the menu load
        (void) g_menu_model_get_n_items(m_menuModel);
        m_signalChangedId = g_signal_connect(m_menuModel,
                                             "items-changed",
                                             G_CALLBACK(QMenuModel::onItemsChanged),
                                             this);
    }

    if (notify) {
        endResetModel();
    }
}

/*! \internal */
GMenuModel *QMenuModel::menuModel() const
{
    return m_menuModel;
}

/*! \internal */
QVariant QMenuModel::data(const QModelIndex &index, int role) const
{
    QVariant attribute;
    int rowCountValue = rowCount();

    if ((rowCountValue > 0) && (index.row() >= 0) && (index.row() < rowCountValue)) {
        if (m_menuModel) {
            switch (role) {
            case Action:
                attribute = getStringAttribute(index, G_MENU_ATTRIBUTE_ACTION);
                break;
            case Label:
                attribute = getStringAttribute(index, G_MENU_ATTRIBUTE_LABEL);
                break;
            case LinkSection:
                attribute = getLink(index, G_MENU_LINK_SECTION);
                break;
            case LinkSubMenu:
                attribute = getLink(index, G_MENU_LINK_SUBMENU);
                break;
            case Extra:
                attribute = getExtraProperties(index);
                break;
            default:
                break;
            }
        }
    }
    return attribute;
}

/*! \internal */
QModelIndex QMenuModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

/*! \internal */
int QMenuModel::rowCount(const QModelIndex &) const
{
    if (m_menuModel) {
        return g_menu_model_get_n_items(m_menuModel);
    }
    return 0;
}

/*! \internal */
QVariant QMenuModel::getStringAttribute(const QModelIndex &index,
                                        const QString &attribute) const
{
    QVariant result;
    gchar* value = NULL;
    g_menu_model_get_item_attribute(m_menuModel,
                                    index.row(),
                                    attribute.toLatin1(),
                                    "s", &value);
    if (value) {
        result = QVariant(QString::fromLatin1(value));
        g_free(value);
    }
    return result;
}

/*! \internal */
QVariant QMenuModel::getLink(const QModelIndex &index,
                             const QString &linkName) const
{
    GMenuModel *link;

    link = g_menu_model_get_item_link(m_menuModel,
                                      index.row(),
                                      linkName.toLatin1());

    if (link) {      
        QMenuModel *other = new QMenuModel(link, const_cast<QMenuModel*>(this));
        return QVariant::fromValue<QObject*>(other);
    }

    return QVariant();
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

/*! \internal */
QVariant QMenuModel::getExtraProperties(const QModelIndex &index) const
{
    GMenuAttributeIter *iter = g_menu_model_iterate_item_attributes(m_menuModel, index.row());
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
void QMenuModel::onItemsChanged(GMenuModel *,
                                gint position,
                                gint removed,
                                gint added,
                                gpointer data)
{
    QMenuModel *self = reinterpret_cast<QMenuModel*>(data);

    if (removed > 0) {
        self->beginRemoveRows(QModelIndex(), position, position + removed - 1);
        self->endRemoveRows();
    }

    if (added > 0) {
        self->beginInsertRows(QModelIndex(), position, position + added - 1);
        self->endInsertRows();
    }
}

