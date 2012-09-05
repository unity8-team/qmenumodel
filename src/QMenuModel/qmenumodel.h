#ifndef QMENUMODEL_H
#define QMENUMODEL_H

#include <QAbstractListModel>
#include <gio/gio.h>


class QMenuModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum MenuRoles {
        Action = 0,
        Label,
        LinkSection,
        LinkSubMenu
    };

    ~QMenuModel();

    /* QAbstractItemModel */
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QModelIndex parent (const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

protected:
    QMenuModel(QObject *parent=0, GMenuModel *other=0);
    void setMenuModel(GMenuModel *model);
    GMenuModel *menuModel() const;

private:
    GMenuModel *m_menuModel;
    guint m_signalChangedId;

    QVariant getStringAttribute(const QModelIndex &index, const QString &attribute) const;
    QVariant getLink(const QModelIndex &index, const QString &linkName) const;

    static void onItemsChanged(GMenuModel *model, gint position, gint removed, gint added, gpointer data);

};

#endif
