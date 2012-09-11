#include "qmenumodel.h"

#include <QDebug>

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
    setMenuModel(other);
}

QMenuModel::~QMenuModel()
{
   setMenuModel(NULL);
}

void QMenuModel::setMenuModel(GMenuModel *other)
{
    if (m_menuModel == other) {
        return;
    }

    beginResetModel();

    if (m_menuModel) {
        g_signal_handler_disconnect(m_menuModel, m_signalChangedId);
        m_signalChangedId = 0;
        g_object_unref(m_menuModel);
    }

    m_menuModel = other;    

    if (m_menuModel) {
        // this will trigger the menu load
        (void) g_menu_model_get_n_items(m_menuModel);
        //qDebug() << "Menu size:" << g_menu_model_get_n_items(m_menuModel);
        m_signalChangedId = g_signal_connect(m_menuModel,
                                             "items-changed",
                                             G_CALLBACK(QMenuModel::onItemsChanged),
                                             this);
    }

    endResetModel();
}

GMenuModel *QMenuModel::menuModel() const
{
    return m_menuModel;
}

/* QAbstractItemModel */
int QMenuModel::columnCount(const QModelIndex &) const
{
    return 1;
}

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

QModelIndex QMenuModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

int QMenuModel::rowCount(const QModelIndex &) const
{
    if (m_menuModel) {
        return g_menu_model_get_n_items(m_menuModel);
    }
    return 0;
}

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

QVariant QMenuModel::getExtraProperties(const QModelIndex &index) const
{
    GMenuAttributeIter *iter = g_menu_model_iterate_item_attributes(m_menuModel, index.row());
    if (iter == NULL) {
        return QVariant();
    }

    QObject *extra = new QObject(const_cast<QMenuModel*>(this));
    const gchar *attrName = NULL;
    GVariant *value = NULL;
    while (g_menu_attribute_iter_get_next (iter, &attrName, &value)) {
        qDebug() << "Set property:" << attrName;
        if (strncmp("x-", attrName, 2) == 0) {
            extra->setProperty(attrName, parseGVariant(value));
        }
    }

    return QVariant::fromValue<QObject*>(extra);
}


void QMenuModel::onItemsChanged(GMenuModel *,
                                gint position,
                                gint removed,
                                gint added,
                                gpointer data)
{
    QMenuModel *self = reinterpret_cast<QMenuModel*>(data);
    //qDebug() << "Item Changed" << position << removed << added;

    if (removed > 0) {
        self->beginRemoveRows(QModelIndex(), position, position + removed - 1);
        self->endRemoveRows();
    }

    if (added > 0) {
        self->beginInsertRows(QModelIndex(), position, position + added - 1);
        self->endInsertRows();
    }
}

QVariant QMenuModel::parseGVariant(GVariant *value)
{
    QVariant result;
    if (value == NULL) {
        return result;
    }

    const GVariantType *type = g_variant_get_type(value);
    if (g_variant_type_equal(type, G_VARIANT_TYPE_BOOLEAN)) {
        result.setValue((bool)g_variant_get_boolean(value));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_BYTE)) {
        result.setValue(g_variant_get_byte(value));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_INT16)) {
        result.setValue(g_variant_get_int16(value));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_UINT16)) {
        result.setValue(g_variant_get_uint16(value));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_INT32)) {
        result.setValue(g_variant_get_int32(value));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_UINT32)) {
        result.setValue(g_variant_get_uint32(value));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_INT64)) {
        result.setValue(g_variant_get_int64(value));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_UINT64)) {
        result.setValue(g_variant_get_uint64(value));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_DOUBLE)) {
        result.setValue(g_variant_get_double(value));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_STRING)) {
        gsize size = 0;
        const gchar *v = g_variant_get_string(value, &size);
        result.setValue(QString::fromLatin1(v, size));
    } else {
        qWarning() << "Unsupported GVariant value";
    }

    /* TODO: implement convertions to others types
     * G_VARIANT_TYPE_HANDLE
     * G_VARIANT_TYPE_OBJECT_PATH
     * G_VARIANT_TYPE_SIGNATURE
     * G_VARIANT_TYPE_VARIANT
     * G_VARIANT_TYPE_ANY
     * G_VARIANT_TYPE_BASIC
     * G_VARIANT_TYPE_MAYBE
     * G_VARIANT_TYPE_ARRAY
     * G_VARIANT_TYPE_TUPLE
     * G_VARIANT_TYPE_UNIT
     * G_VARIANT_TYPE_DICT_ENTRY
     * G_VARIANT_TYPE_DICTIONARY
     * G_VARIANT_TYPE_STRING_ARRAY
     * G_VARIANT_TYPE_BYTESTRING
     * G_VARIANT_TYPE_OBJECT_PATH_ARRAY
     * G_VARIANT_TYPE_BYTESTRING_ARRAY
     * G_VARIANT_TYPE_VARDICT
     */

    return result;
}

