
#include "dbusmenuscript.h"
#include <QObject>
#include <QtTestGui>
#include <QDebug>

#define WAIT_TIMEOUT   500

DBusMenuScript::DBusMenuScript()
    :m_script(0)
{
}

DBusMenuScript::~DBusMenuScript()
{
    quit();
}

bool DBusMenuScript::connect()
{
    QTest::qWait(WAIT_TIMEOUT);
    m_script = new QDBusInterface(SCRIPT_SERVICE_NAME,
                                  SCRIPT_OBJECT_PATH,
                                  SCRIPT_INTERFACE_NAME,
                                  QDBusConnection::sessionBus(), 0);
    if (m_script && m_script->isValid()) {
        return true;
    } else {
        qWarning() << "DBUS ERROR:" << m_script->lastError().message();
        return false;
    }
}

void DBusMenuScript::publishMenu()
{
    if (m_script) {
        m_script->call("publishMenu");
        QTest::qWait(WAIT_TIMEOUT);
    }
}

void DBusMenuScript::unpublishMenu()
{
    if (m_script) {
        m_script->call("unpublishMenu");
        QTest::qWait(WAIT_TIMEOUT);
    }
}


void DBusMenuScript::quit()
{
    if (m_script) {
        m_script->call("quit");
        QTest::qWait(WAIT_TIMEOUT);
        delete m_script;
        m_script = 0;
    }
}

void DBusMenuScript::walk(int steps)
{
    if (m_script) {
        m_script->call("walk", steps);
        QTest::qWait(WAIT_TIMEOUT);
    }
}

void DBusMenuScript::run()
{
    if (m_script) {
        m_script->call("walk", -1);
        QTest::qWait(WAIT_TIMEOUT);
    }
}
