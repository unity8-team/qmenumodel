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
#include "converter.h"

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
      m_signalChangedId(0),
      m_rowCount(0),
      m_currentOperationPosition(0),
      m_currentOperationAdded(0),
      m_currentOperationRemoved(0)
{
    m_cache = new QHash<int, QMenuModel*>;
    setMenuModel(other);

    connect(this, SIGNAL(rowsInserted(const QModelIndex &, int, int)), SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), SIGNAL(countChanged()));
    connect(this, SIGNAL(modelReset()), SIGNAL(countChanged()));
}

/*! \internal */
QMenuModel::~QMenuModel()
{
    clearModel(true);
    delete m_cache;
}

/*!
    \qmlmethod QDBusMenuModel::get(int)

    Returns the item at index in the model. This allows the item data to be accessed from JavaScript:

    \b Note: methods should only be called after the Component has completed.
*/

QVariantMap QMenuModel::get(int row) const
{
    QVariantMap result;

    QModelIndex index = this->index(row);
    if (index.isValid()) {
        QMap<int, QVariant> data = itemData(index);
        const QHash<int, QByteArray> roleNames = this->roleNames();
        Q_FOREACH(int i, roleNames.keys()) {
            result.insert(roleNames[i], data[i]);
        }
    }
    return result;
}

/*!
    \qmlmethod QDBusMenuModel::count()

    The number of data entries in the model.

    \b Note: methods should only be called after the Component has completed.
*/
int QMenuModel::count() const
{
    return m_rowCount;
}

/*! \internal */
void QMenuModel::setMenuModel(GMenuModel *other)
{
    if (m_menuModel == other) {
        return;
    }

    beginResetModel();

    clearModel();

    m_menuModel = other;

    if (m_menuModel) {
        g_object_ref(m_menuModel);
        m_rowCount = g_menu_model_get_n_items(m_menuModel);
        m_signalChangedId = g_signal_connect(m_menuModel,
                                             "items-changed",
                                             G_CALLBACK(QMenuModel::onItemsChanged),
                                             this);
    } else {
        m_rowCount = 0;
    }

    endResetModel();
}

/*! \internal */
GMenuModel *QMenuModel::menuModel() const
{
    return m_menuModel;
}

/*! \internal */
void QMenuModel::clearModel(bool destructor)
{
    if (m_menuModel) {
        g_signal_handler_disconnect(m_menuModel, m_signalChangedId);
        m_signalChangedId = 0;
        g_object_unref(m_menuModel);
        m_menuModel = NULL;
    }

    Q_FOREACH(QMenuModel* child, *m_cache) {
        // avoid emit signals during the object destruction this can crash qml
        if (!destructor) {
            child->setMenuModel(NULL);
        }
        child->deleteLater();
    }
    m_cache->clear();
}

/*! \internal */
QHash<int, QByteArray> QMenuModel::roleNames() const
{
    static QHash<int, QByteArray> roles;
    if (roles.isEmpty()) {
        roles[Action] = "action";
        roles[Label] = "label";
        roles[LinkSection] = "linkSection";
        roles[LinkSubMenu] = "linkSubMenu";
        roles[Extra] = "extra";
    }
    return roles;
}

/*! \internal */
QVariant QMenuModel::data(const QModelIndex &index, int role) const
{
    QVariant attribute;

    // Return a empty variant if the remove operation is in progress
    if ((m_currentOperationRemoved > 0) &&
        (index.row() >= m_currentOperationPosition) &&
        (index.row() < (m_currentOperationPosition + m_currentOperationRemoved))) {
        return attribute;
    }

    int rowCountValue = rowCount() + (m_currentOperationAdded - m_currentOperationRemoved);
    int row = rowIndex(index);

    if ((row >= 0) && (row < rowCountValue)) {
        if (m_menuModel) {
            switch (role) {
            case Action:
                attribute = getStringAttribute(row, G_MENU_ATTRIBUTE_ACTION);
                break;
            case Label:
                attribute = getStringAttribute(row, G_MENU_ATTRIBUTE_LABEL);
                break;
            case LinkSection:
                attribute = getLink(row, G_MENU_LINK_SECTION);
                break;
            case LinkSubMenu:
                attribute = getLink(row, G_MENU_LINK_SUBMENU);
                break;
            case Extra:
                attribute = getExtraProperties(row);
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
    return m_rowCount;
}

/*! \internal */
int QMenuModel::rowIndex(const QModelIndex &index) const
{
    int row = index.row();
    /*
    if ((m_currentOperationAdded > 0) && (row >= m_currentOperationPosition)) {
        row += m_currentOperationAdded;
    } else if ((m_currentOperationRemoved > 0) && (row >= (row >= m_currentOperationPosition))) {

    }
    */
    if (row >= m_currentOperationPosition) {
        row += (m_currentOperationAdded - m_currentOperationRemoved);
    }
    return row;
}


/*! \internal */
QVariant QMenuModel::getStringAttribute(int row,
                                        const QString &attribute) const
{
    QVariant result;
    gchar* value = NULL;
    g_menu_model_get_item_attribute(m_menuModel,
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
QVariant QMenuModel::getLink(int row,
                             const QString &linkName) const
{
    GMenuModel *link = g_menu_model_get_item_link(m_menuModel,
                                                  row,
                                                  linkName.toUtf8().data());
    if (link) {
        QMenuModel* child = 0;
        if (m_cache->contains(row)) {
            child = m_cache->value(row);
            if (child->menuModel() != link) {
                child->setMenuModel(link);
            }
        }
        if (child == 0) {
            child = new QMenuModel(link);
            m_cache->insert(row, child);
        }
        g_object_unref(link);
        return QVariant::fromValue<QObject*>(child);
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
QVariant QMenuModel::getExtraProperties(int row) const
{
    GMenuAttributeIter *iter = g_menu_model_iterate_item_attributes(m_menuModel, row);
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
QHash<int, QMenuModel*> QMenuModel::cache() const
{
    return *m_cache;
}

/*! \internal */
void QMenuModel::onItemsChanged(GMenuModel *model,
                                gint position,
                                gint removed,
                                gint added,
                                gpointer data)
{
    QMenuModel *self = reinterpret_cast<QMenuModel*>(data);
    QHash<int, QMenuModel*>* cache = self->m_cache;

    self->m_currentOperationPosition = position;
    self->m_currentOperationAdded = added;
    self->m_currentOperationRemoved = removed;

    int prevcount = g_menu_model_get_n_items(model) + removed - added;
    if (removed > 0) {
        // help function to proxy model
        for(int i=position, iMax=(position + removed - 1); i <= iMax; i++) {
            if (cache->contains(i)) {
                Q_EMIT self->aboutToRemoveLink(cache->value(i), i);
            }
        }

        // We need clear the removed items before start remove it,
        // because we do not have information about the previous data, and QML
        // will try to retrieve it during the signal 'rowsAboutToBeRemoved'
        QModelIndex start = self->index(position);
        QModelIndex end = self->index(position + removed -1);
        Q_EMIT self->dataChanged(start, end);

        self->beginRemoveRows(QModelIndex(), position, position + removed - 1);
        self->m_currentOperationPosition = self->m_currentOperationAdded = self->m_currentOperationRemoved = 0;
        self->m_rowCount -= removed;
        // Remove invalidated menus from the cache
        for (int i = position, iMax = position + removed; i < iMax; ++i) {
            if (cache->contains(i)) {
                QMenuModel *cached = cache->take(i);
                cached->setMenuModel(NULL);
                cached->deleteLater();
            }
        }
        // Update the indexes of other cached menus to account for the removals
        for (int i = position + removed; i < prevcount; ++i) {
            if (cache->contains(i)) {
                cache->insert(i - removed, cache->take(i));
            }
        }
        self->endRemoveRows();
    }

    if (added > 0) {
        self->beginInsertRows(QModelIndex(), position, position + added - 1);
        self->m_currentOperationPosition = self->m_currentOperationAdded = self->m_currentOperationRemoved = 0;
        // Update the indexes of cached menus to account for the insertions
        for (int i = prevcount - removed - 1; i >= position; --i) {
            if (cache->contains(i)) {
                cache->insert(i + added, cache->take(i));
            }
        }
        self->m_rowCount += added;
        self->endInsertRows();
    }
}

