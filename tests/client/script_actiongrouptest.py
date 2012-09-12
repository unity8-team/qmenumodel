#!/usr/bin/python2.7

import time
from gi.repository import GLib
from menuscript import Script, ActionList, MENU_OBJECT_PATH

al = ActionList(MENU_OBJECT_PATH)
al.appendItem("Menu0", "Menu0Act")
al.appendItem("Menu1", "Menu1Act")
al.removeItem("1", "Menu1Act")

t = Script.create(al)
t.run()

