#ifndef QDBUSOBJECT_H
#define QDBUSOBJECT_H

#include <QObject>

#include <gio/gio.h>

class QDbusObject
{
public:
    enum BusType {
        None = 0,
        SessionBus,
        SystemBus,
        LastBusType
    };

    enum ConnectionStatus {
        Disconnected = 0,
        Connecting,
        Connected
    };

    QDbusObject();
    ~QDbusObject();

    BusType busType() const;
    void setBusType(BusType type);

    QString busName() const;
    void setBusName(const QString &busName);

    QString objectPath() const;
    void setObjectPath(const QString &busName);

    ConnectionStatus status() const;

    void connect();
    void disconnect();

protected:
    virtual void serviceAppear(GDBusConnection *connection) = 0;
    virtual void serviceVanish(GDBusConnection *connection) = 0;

    // notify functions
    virtual void busTypeChanged(BusType type) = 0;
    virtual void busNameChanged(const QString &busNameChanged) = 0;
    virtual void objectPathChanged(const QString &objectPath) = 0;
    virtual void statusChanged(ConnectionStatus status) = 0;

private:
    guint m_watchId;
    BusType m_busType;
    QString m_busName;
    QString m_objectPath;
    ConnectionStatus m_status;

    void setStatus(ConnectionStatus status);

    // glib slots
    static void onServiceAppeared(GDBusConnection *connection, const gchar *name, const gchar *name_owner, gpointer data);
    static void onServiceFanished(GDBusConnection *connection, const gchar *name, gpointer data);
};

#endif
