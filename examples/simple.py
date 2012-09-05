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

def action_activated (action, parameter):
    print action.get_name ()

actions = Gio.SimpleActionGroup ()
for i in ['one', 'two', 'three']:
    action = Gio.SimpleAction.new (i, None)
    action.connect ('activate', action_activated)
    actions.insert (action)

numbers = Gio.Menu ()
numbers.append ('One', 'one')
numbers.append ('Two', 'two')
numbers.append ('Three', 'three')

menu = Gio.Menu ()
menu.append ('Menu item', 'one')
menu.append_section ('Numbers', numbers)
menu.append_submenu ('Submenu', numbers)

# export the menu and action group on d-bus
bus = Gio.bus_get_sync (Gio.BusType.SESSION, None)
bus.export_menu_model ('/menu', menu)
bus.export_action_group ('/menu', actions)
print bus.get_unique_name ()

loop = GLib.MainLoop ()
loop.run ()

