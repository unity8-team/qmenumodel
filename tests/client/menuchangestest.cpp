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

extern "C" {
#include <glib-object.h>
}

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
        g_type_init();
        Q_ASSERT(m_script.connect());
    }

    void cleanupTestCase()
    {
        m_script.quit();
    }

    void init()
    {
        m_model.stop();
        m_model.setBusType(DBusEnums::SessionBus);
        m_model.setBusName(MENU_SERVICE_NAME);
        m_model.setObjectPath(MENU_OBJECT_PATH);
    }

    void cleanup()
    {
        m_script.unpublishMenu();
    }

    /*
     * Test it the model updates correct after remove or add a new menu
     */
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
