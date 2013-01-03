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
 *      Olivier Tilloy <olivier.tilloy@canonical.com>
 */

#include "qmenumodel.h"

extern "C" {
#include <gio/gio.h>
}

#include <QtTest>
#include <QTreeView>
#include <QFileSystemModel>


class TestModel : public QMenuModel
{
    Q_OBJECT
public:
    TestModel() : QMenuModel(0)
    {
        GMenu *menu5 = g_menu_new();
        g_menu_append(menu5, "menu6", NULL);
        g_menu_append(menu5, "menu7", NULL);

        GMenu *menu3 = g_menu_new();
        g_menu_append(menu3, "menu4", NULL);
        g_menu_append_section(menu3, "menu5", G_MENU_MODEL(menu5));
        g_menu_append(menu3, "menu8", NULL);

        GMenu *menu = g_menu_new();
        g_menu_append(menu, "menu0", NULL);
        g_menu_append(menu, "menu1", NULL);
        g_menu_append(menu, "menu2", NULL);
        g_menu_append_section(menu, "menu3", G_MENU_MODEL(menu3));

        setMenuModel(G_MENU_MODEL(menu));

        m_menus << menu << menu3 << menu5;
    }

private:
    QList<GMenu*> m_menus;
};

class TreeViewTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        g_type_init();
    }

    void testView()
    {
        TestModel menu;
        //QFileSystemModel menu; // = new QFileSystemModel;
        //menu.setRootPath(QDir::currentPath());
        QTreeView *tree = new QTreeView;
        tree->setModel(&menu);
        tree->show();
        QApplication::exec();
    }

};

QTEST_MAIN(TreeViewTest)

#include "treeviewtest.moc"

