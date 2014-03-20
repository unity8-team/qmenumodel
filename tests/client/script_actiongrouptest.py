#!/usr/bin/env python3
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

import time
from gi.repository import GLib
from menuscript import Script, ActionList, MENU_OBJECT_PATH
from gi._gi import variant_type_from_string

al = ActionList(MENU_OBJECT_PATH)
al.appendItem("Menu0", "Menu0Act", actionStateType=variant_type_from_string('s'), actionState=GLib.Variant('s', 'Menu0State'))
al.appendItem("Menu1", "Menu1Act", actionStateType=variant_type_from_string('s'), actionState=GLib.Variant('s', 'Menu0State'))
al.removeItem("1", "Menu1Act")

t = Script.create(al)
t.run()

