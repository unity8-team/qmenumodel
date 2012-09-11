#include "qdbusmenumodel.h"
#include "dbusmenuscript.h"

#include <QObject>
#include <QSignalSpy>
#include <QtTestGui>
#include <QDebug>

class MenuChangesTest : public QObject
{
    Q_OBJECT
private:
    DBusMenuScript m_script;
    QDBusMenuModel m_model;

private Q_SLOTS:
    void initTestCase()
    {
        Q_ASSERT(m_script.connect());
    }

    void cleanupTestCase()
    {
        m_script.quit();
    }

    void init()
    {
        m_model.stop();
        m_model.setBusType(QDBusObject::SessionBus);
        m_model.setBusName(MENU_SERVICE_NAME);
        m_model.setObjectPath(MENU_OBJECT_PATH);
    }

    void cleanup()
    {
        m_script.unpublishMenu();
    }

    void testMenuItemAppend()
    {
        m_script.publishMenu();
        m_model.start();

        // Create first Item
        m_script.walk();
        QCOMPARE(m_model.rowCount(), 1);

        // Create second item
        m_script.walk();
        QCOMPARE(m_model.rowCount(), 2);

        // Remove item0
        m_script.walk();
        QCOMPARE(m_model.rowCount(), 1);

        // Remove item1
        m_script.walk();
        QCOMPARE(m_model.rowCount(), 0);
    }
};

QTEST_MAIN(MenuChangesTest)

#include "menuchangestest.moc"
