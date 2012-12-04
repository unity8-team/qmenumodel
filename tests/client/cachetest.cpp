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

#include "qmenumodel.h"

extern "C" {
#include <gio/gio.h>
}

#include <QtTest>


class MenuModelTestClass : public QMenuModel
{
    Q_OBJECT
public:
    MenuModelTestClass()
        : QMenuModel(0)
    {
        GMenu *menu3 = g_menu_new();
        g_menu_append(menu3, "menu4", NULL);
        g_menu_append(menu3, "menu5", NULL);
        g_menu_append(menu3, "menu6", NULL);

        GMenu *menu = g_menu_new();
        g_menu_append(menu, "menu0", NULL);
        g_menu_append(menu, "menu1", NULL);
        g_menu_append(menu, "menu2", NULL);
        g_menu_append_section(menu, "menu3", G_MENU_MODEL(menu3));

        setMenuModel(G_MENU_MODEL(menu));

        m_menus << menu << menu3;
    }

    void removeItem(int section, int index)
    {
        GMenu *menu = m_menus[section];
        g_menu_remove(menu, index);
    }

    void insertItem(int section, int index, const QString &label)
    {
        GMenu *menu = m_menus[section];
        g_menu_insert(menu, index, label.toUtf8().data(), NULL);
    }

    int cacheSize() const
    {
        return cache().size();
    }

private:
    QList<GMenu*> m_menus;
};

class CacheTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        g_type_init();
    }

    //
    // Test if the link property always returns the same element
    //
    void testStaticMenuCache()
    {
        MenuModelTestClass menu;

        QModelIndex index = menu.index(3);

        QVariant data = menu.data(index, QMenuModel::LinkSection);
        QCOMPARE(menu.cacheSize(), 1);

        QVariant data2 = menu.data(index, QMenuModel::LinkSection);
        QCOMPARE(menu.cacheSize(), 1);

        QVERIFY(data.value<QObject*>() == data2.value<QObject*>());

        QMenuModel *section = qvariant_cast<QMenuModel*>(data);

        index = section->index(1);
        data = menu.data(index, QMenuModel::LinkSection);
        data2 = menu.data(index, QMenuModel::LinkSection);
        QVERIFY(data.value<QObject*>() == data2.value<QObject*>());
    }


    //
    // Test if cache works after add a new item
    //
    void testAddItem()
    {
        MenuModelTestClass menu;

        QModelIndex index = menu.index(3);
        QVariant data = menu.data(index, QMenuModel::LinkSection);

        menu.insertItem(0, 1, "newMenu");

        index = menu.index(4);
        QVariant data2 = menu.data(index, QMenuModel::LinkSection);

        QCOMPARE(menu.cacheSize(), 1);
        QVERIFY(data.value<QObject*>() == data2.value<QObject*>());
    }


    //
    // Test if cache works after remove a item
    //
    void testRemoveItem()
    {
        MenuModelTestClass menu;

        QModelIndex index = menu.index(3);
        QVariant data = menu.data(index, QMenuModel::LinkSection);

        menu.removeItem(0, 1);

        index = menu.index(2);
        QVariant data2 = menu.data(index, QMenuModel::LinkSection);

        QCOMPARE(menu.cacheSize(), 1);
        QVERIFY(data.value<QObject*>() == data2.value<QObject*>());
    }

    //
    // Test if cached item is removed after removed from the menu
    //
    void testRemoveCachedItem()
    {
        MenuModelTestClass menu;

        QModelIndex index = menu.index(3);
        QVariant data = menu.data(index, QMenuModel::LinkSection);

        QCOMPARE(menu.cacheSize(), 1);
        menu.removeItem(0, 3);
        QCOMPARE(menu.cacheSize(), 0);
    }
};

QTEST_MAIN(CacheTest)

#include "cachetest.moc"

