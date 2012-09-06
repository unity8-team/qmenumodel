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

    endResetModel();

    if (m_menuModel) {
        qDebug() << "Menu size:" << g_menu_model_get_n_items(m_menuModel);
        m_signalChangedId = g_signal_connect(m_menuModel,
                                             "items-changed",
                                             G_CALLBACK(QMenuModel::onItemsChanged),
                                             this);
    }
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
            switch (role)
            {
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

