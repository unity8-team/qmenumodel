#!/usr/bin/env python3

import time
from gi.repository import GLib
from menuscript import Script, ActionList, MENU_OBJECT_PATH
from gi._gi import variant_type_from_string

al = ActionList(MENU_OBJECT_PATH)
al.appendItem("Menu0", "Menu0Act", actionStateType=variant_type_from_string('s'))
al.appendItem("Menu1", "Menu1Act", actionStateType=variant_type_from_string('s'))
al.removeItem("1", "Menu1Act")

t = Script.create(al)
t.run()

