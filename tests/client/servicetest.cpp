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

#include "qdbusmenumodel.h"
#include "dbusmenuscript.h"

#include <QObject>
#include <QSignalSpy>
#include <QtTestGui>
#include <QDebug>

class ServiceTest : public QObject
{
    Q_OBJECT
private:
    DBusMenuScript m_script;
    QDBusMenuModel m_model;

    void setupModel(QDBusMenuModel *model)
    {
        model->setBusType(QDBusObject::SessionBus);
        model->setBusName(MENU_SERVICE_NAME);
        model->setObjectPath(MENU_OBJECT_PATH);
    }

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

    void testMenuStartStopWithNoService()
    {
        m_model.start();
        QCOMPARE(m_model.status(), QDBusObject::Connecting);

        m_model.stop();
        QCOMPARE(m_model.status(), QDBusObject::Disconnected);
    }

    void testMenuStartStopWithService()
    {
        // Make menu available
        m_script.publishMenu();

        // start model
        m_model.start();

        // Wait for dbus sync
        QTest::qWait(500);

        QCOMPARE(m_model.status(), QDBusObject::Connected);

        // Diconnect model
        m_model.stop();
        QCOMPARE(m_model.status(), QDBusObject::Disconnected);
    }

    void testMenuServiceAppearAndDissapear()
    {
        m_model.start();
        QCOMPARE(m_model.status(), QDBusObject::Connecting);

        QSignalSpy spy(&m_model, SIGNAL(statusChanged(QDBusObject::ConnectionStatus)));

        // Make menu available
        m_script.publishMenu();

        // singal changed to connected
        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_model.status(), QDBusObject::Connected);

        // remove menu service
        m_script.unpublishMenu();

        // signal changed to connecting
        QCOMPARE(spy.count(), 2);
        QCOMPARE(m_model.status(), QDBusObject::Connecting);
    }
};

QTEST_MAIN(ServiceTest)

#include "servicetest.moc"
