# creates this menu:
#
#   Menu Item
#   ----------------
#   One
#   Two
#   Three
#   ----------------
#   Submenu        > | One
#                    | Two
#                    | Three

from gi.repository import GLib, Gio

def on_items_changed (model, position, removed, added, data):
    print 'items changed:', position, removed, added

bus = Gio.bus_get_sync (Gio.BusType.SESSION, None)
print dir(bus)
menu = bus.get_menu_model(':1.473', '/menu')
#menu = Gio.dbus_menu_model_get(Gio.BusType.SESSION, ':1.473', '/menu')
menu.connect ('items-changed', on_items_changed)

loop = GLib.MainLoop ()
loop.run ()

