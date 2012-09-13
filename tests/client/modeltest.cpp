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

class ModelTest : public QObject
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

    void testColumnCount()
    {
        QCOMPARE(m_model.columnCount(), 1);
    }

    void testParent()
    {
        QCOMPARE(m_model.parent(QModelIndex()), QModelIndex());
    }

    void testBusTypeProperty()
    {
        m_model.setProperty("busType", 1);
        QCOMPARE(m_model.busType(), QDBusObject::SessionBus);

        m_model.setProperty("busType", 2);
        QCOMPARE(m_model.busType(), QDBusObject::SystemBus);

        m_model.setProperty("busType", 0);
        QCOMPARE(m_model.busType(), QDBusObject::SystemBus);

        m_model.setProperty("busType", 10);
        QCOMPARE(m_model.busType(), QDBusObject::SystemBus);

    }

    void testData()
    {
        // Make menu available
        m_script.publishMenu();
        m_script.run();

        // start model
        m_model.start();

        // Wait for dbus sync
        QTest::qWait(500);

        QCOMPARE(m_model.status(), QDBusObject::Connected);
        QCOMPARE(m_model.rowCount(), 4);

        // Label (String)
        QVariant label = m_model.data(m_model.index(0, 0), QMenuModel::Label);
        QVERIFY(label.isValid());
        QCOMPARE(label.type(), QVariant::String);
        QCOMPARE(label.toString(), QString("Menu0"));

        // Action (String)
        QVariant action = m_model.data(m_model.index(1, 0), QMenuModel::Action);
        QVERIFY(action.isValid());
        QCOMPARE(action.type(), QVariant::String);
        QCOMPARE(action.toString(), QString("Menu1Act"));

        // Section (QObject)
        QVariant vSection = m_model.data(m_model.index(2, 0), QMenuModel::LinkSection);
        QVERIFY(vSection.isValid());
        QMenuModel *section = qobject_cast<QMenuModel*>(vSection.value<QObject*>());
        QVERIFY(section);
        QCOMPARE(section->rowCount(), 2);

        // SubMenu (QObject)
        QVariant vSubMenu = m_model.data(m_model.index(3, 0), QMenuModel::LinkSubMenu);
        QVERIFY(vSubMenu.isValid());
        QMenuModel *submenu = qobject_cast<QMenuModel*>(vSubMenu.value<QObject*>());
        QVERIFY(submenu);

        // Wait for menu load (submenus are loaded async)
        QTest::qWait(500);
        QCOMPARE(submenu->rowCount(), 2);
    }

    void testExtraData()
    {
        // Make menu available
        m_script.publishMenu();
        m_script.run();

        // start model
        m_model.start();

        // Wait for dbus sync
        QTest::qWait(500);

        QCOMPARE(m_model.status(), QDBusObject::Connected);
        QCOMPARE(m_model.rowCount(), 4);

        QVariant e = m_model.data(m_model.index(0, 0), QMenuModel::Extra);
        QVERIFY(e.isValid());
        QVariantMap extra = e.toMap();

        // Boolean
        QVariant v = extra["boolean"];
        QCOMPARE(v.type(), QVariant::Bool);
        QCOMPARE(v.toBool(), true);

        // Byte
        v = extra["byte"];
        QCOMPARE(v.typeName(), "uchar");
        QCOMPARE(v.value<uchar>(), (uchar)42);

        // Int16
        v = extra["int16"];
        QCOMPARE(v.typeName(), "short");
        QCOMPARE(v.value<short>(), (short)-42);

        // UInt16
        v = extra["uint16"];
        QCOMPARE(v.typeName(), "ushort");
        QCOMPARE(v.value<ushort>(), (ushort)42);

        // Int32
        v = extra["int32"];
        QCOMPARE(v.type(), QVariant::Int);
        QCOMPARE(v.toInt(), -42);

        // UInt32
        v = extra["uint32"];
        QCOMPARE(v.type(), QVariant::UInt);
        QCOMPARE(v.toUInt(), (uint) 42);

        // Int64
        v = extra["int64"];
        QCOMPARE(v.typeName(), "long");
        QCOMPARE(v.value<long>(), (long) -42);

        // UInt64
        v = extra["uint64"];
        QCOMPARE(v.typeName(), "ulong");
        QCOMPARE(v.value<ulong>(), (ulong) 42);

        // Double
        v = extra["double"];
        QCOMPARE(v.type(), QVariant::Double);
        QCOMPARE(v.toDouble(), 42.42);

        // String
        v = extra["string"];
        QCOMPARE(v.type(), QVariant::String);
        QCOMPARE(v.toString(), QString("42"));
    }

};

QTEST_MAIN(ModelTest)

#include "modeltest.moc"
