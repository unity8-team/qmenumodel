#!/usr/bin/env python3
# -*- encoding: utf-8 -*-
#
#  Copyright 2013 Canonical Ltd.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License as published by
#  the Free Software Foundation; version 3.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

"""
This example script exports a menu model on the session bus under the name
com.canonical.testmenu and at the object path /com/canonical/testmenu.
The menu model contains items that have attributes with custom values, as well
as sub-menus.
"""

import sys

from gi.repository import Gio
from gi.repository import GLib


BUS_NAME = 'com.canonical.testmenu'
BUS_OBJECT_PATH = '/com/canonical/testmenu'


if __name__ == '__main__':
    bus = Gio.bus_get_sync(Gio.BusType.SESSION, None)
    # Claim well-known bus name and ensure only one instance of self is running
    # at any given time.
    # http://dbus.freedesktop.org/doc/dbus-specification.html#message-bus-names
    proxy = Gio.DBusProxy.new_sync(bus, 0, None,
                                   'org.freedesktop.DBus',
                                   '/org/freedesktop/DBus',
                                   'org.freedesktop.DBus', None)
    result = proxy.RequestName('(su)', BUS_NAME, 0x4)
    if result != 1 :
        print >> sys.stderr, ("Name '%s' is already owned on the session bus."
                              "Aborting.") % BUS_NAME
        sys.exit(1)

    menu = Gio.Menu()
    foo = Gio.MenuItem.new('foo', 'app.foo')
    foo.set_attribute_value('x-additionaltext',
                            GLib.Variant.new_string('lorem ipsum'))
    foo.set_attribute_value('x-enabled', GLib.Variant.new_boolean(True))
    menu.append_item(foo)
    bar = Gio.MenuItem.new('bar', 'app.bar')
    bar.set_attribute_value('x-defaultvalue',
                            GLib.Variant.new_string('Hello World!'))
    bar.set_attribute_value('x-canonical-currentvalue',
                            GLib.Variant.new_string('awesome'))
    bar.set_attribute_value('x-velocity', GLib.Variant.new_uint64(83374))
    menu.append_item(bar)
    menu.append('bleh', 'app.bleh')
    submenu = Gio.Menu()
    submenu.append('submenu A', 'app.suba')
    submenu2 = Gio.Menu()
    submenu2.append('submenu2 A', 'app.sub2a')
    submenu2.append('submenu2 B', 'app.sub2b')
    submenu2.append('submenu2 C', 'app.sub2c')
    submenu.append_submenu('submenu submenu', submenu2)
    submenu.append('submenu C', 'app.subc')
    menu.append_submenu('submenu', submenu)
    menu.append('baz', 'app.baz')
    bus.export_menu_model(BUS_OBJECT_PATH, menu)

    GLib.MainLoop().run()

