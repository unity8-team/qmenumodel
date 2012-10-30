#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import time
from gi.repository import GLib
from menuscript import Script, ActionList, MENU_OBJECT_PATH

al = ActionList(MENU_OBJECT_PATH)

# create map
pmap = {'int64' : GLib.Variant('x',  -42),
        'string': GLib.Variant('s',  '42'),
        'double': GLib.Variant('d',  42.42)}

al.appendItem("Menu0", "Menu0Act", None, None, {'x-boolean' : GLib.Variant('b', True),
                                                'x-byte' : GLib.Variant('y', 42),
                                                'x-int16' : GLib.Variant('n', -42),
                                                'x-uint16' : GLib.Variant('q', 42),
                                                'x-int32' : GLib.Variant('i', -42),
                                                'x-uint32' : GLib.Variant('u', 42),
                                                'x-int64' : GLib.Variant('x',  -42),
                                                'x-uint64' : GLib.Variant('t', 42),
                                                'x-double' : GLib.Variant('d', 42.42),
                                                'x-string' : GLib.Variant('s', '42'),
                                                'x-utf8' : GLib.Variant('s', 'dança'),
                                                'x-map' : GLib.Variant('a{sv}', pmap),
                                               })
al.appendItem("Menu1", "Menu1Act")

al.appendItem("Menu2", "Menu2Act", "section")
al.appendItem("Menu2.1", "Menu2.1Act", None, "2")
al.appendItem("Menu2.2", "Menu2.2Act", None, "2")

al.appendItem("Menu3", "Menu3Act", "submenu")
al.appendItem("Menu3.1", "Menu3.1Act", None, "3")
al.appendItem("Menu3.2", "Menu3.2Act", None, "3")

t = Script.create(al)
t.run()

