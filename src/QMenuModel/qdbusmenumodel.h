#ifndef QDBUSMENUMODEL_H
#define QDBUSMENUMODEL_H

#include "qmenumodel.h"

#include <gio/gio.h>

class QDBusMenuModel : public QMenuModel
{
    Q_OBJECT
    Q_PROPERTY(int busType READ busType WRITE setIntBusType NOTIFY busTypeChanged)
    Q_PROPERTY(QString busName READ busName WRITE setBusName NOTIFY busNameChanged)
    Q_PROPERTY(QString objectPath READ objectPath WRITE setObjectPath NOTIFY objectPathChanged)

public:
    enum BusType {
        None = 0,
        SessionBus,
        SystemBus,
        LastBusType
    };

    QDBusMenuModel(QObject *parent=0);
    ~QDBusMenuModel();

    BusType busType() const;
    void setBusType(BusType type);

    QString busName() const;
    void setBusName(const QString &busName);

    QString objectPath() const;
    void setObjectPath(const QString &busName);

    bool isConnected() const;

public Q_SLOTS:
    void connect();
    void disconnect();


Q_SIGNALS:
    void busTypeChanged(BusType type);
    void busNameChanged(const QString &busNameChanged);
    void objectPathChanged(const QString &objectPath);

    void connected();
    void disconnected();
    void connectionError(const QString &errorMessage);

private:
    guint m_watchId;
    BusType m_busType;
    QString m_busName;
    QString m_objectPath;

    // workaround to support busType as int
    void setIntBusType(int busType);

    // glib slots
    static void onServiceAppeared(GDBusConnection *connection, const gchar *name, const gchar *name_owner, gpointer data);
    static void onServiceFanished(GDBusConnection *connection, const gchar *name, gpointer data);
};

#endif
