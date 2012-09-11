#!/usr/bin/python2.7

import time
from menuscript import Script, ActionList, MENU_OBJECT_PATH

al = ActionList(MENU_OBJECT_PATH)
al.appendItem("Menu0", "Menu0")
al.appendItem("Menu1", "Menu1")
al.removeItem(0) # remove Menu0
al.removeItem(0) # remove Menu1

t = Script.create(al)
t.run()

