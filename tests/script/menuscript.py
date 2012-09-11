import dbus
import dbus.service
from dbus import glib
from dbus.mainloop.glib import DBusGMainLoop
from gi.repository import GObject
from gi.repository import GLib
from gi.repository import Gio

SERVICE_NAME     = "com.canonical.test"
INTERFACE_NAME   = "com.canonical.test.menuscript"
OBJECT_PATH      = "/com/canonical/test/menuscript"
MENU_SERVICE_NAME= SERVICE_NAME + ".menu"
MENU_OBJECT_PATH = OBJECT_PATH + "/menu"
bus = None

class Script(dbus.service.Object):
    def __init__(self, aList, session, object_path):
        dbus.service.Object.__init__(self, session, object_path)
        self._list = aList
        self._session  = session

    def run(self):
        self._loop = GObject.MainLoop()
        self._loop.run()

    @dbus.service.method(dbus_interface=INTERFACE_NAME,
                         in_signature='', out_signature='',
                         sender_keyword='sender')
    def publishMenu(self, sender=None):
        self._list.start()

    @dbus.service.method(dbus_interface=INTERFACE_NAME,
                         in_signature='', out_signature='',
                         sender_keyword='sender')
    def unpublishMenu(self, sender=None):
        self._list.stop()

    @dbus.service.method(dbus_interface=INTERFACE_NAME,
                         in_signature='', out_signature='',
                         sender_keyword='sender')
    def quit(self, sender=None):
        self.unpublishMenu();
        self._loop.quit()

    @dbus.service.method(dbus_interface=INTERFACE_NAME,
                         in_signature='i', out_signature='',
                         sender_keyword='sender')
    def walk(self, steps, sender=None):
        if steps == -1 or steps > self._list.size():
            steps = self._list.size()

        while(steps > 0):
            self._list.walk()
            steps -= 1

    @staticmethod
    def create(aList):
        global bus

        GObject.threads_init()
        glib.threads_init()

        dbus_loop = DBusGMainLoop()
        bus = dbus.SessionBus(mainloop=dbus_loop)
        bus_name = dbus.service.BusName(SERVICE_NAME, bus=bus)
        return Script(aList, bus_name, OBJECT_PATH)

class Action(object):
    def __init__(self, aList, action, **kwargs):
        self._list = aList
        self._action = action
        self._kargs = kwargs

    def setProperties(self, menu, props):
        if props:
           for key in props:
               menu.set_attribute_value(key, props[key])

    def appendItem(self):
        parentId = self._kargs['parentId']
        if parentId and len(parentId):
            parent = self._list.getMenu(parentId)[0]
        else:
            parent = self._list._root

        if self._kargs['link'] == None:
            item = Gio.MenuItem.new(self._kargs['label'], self._kargs['actionName'])
            self.setProperties(item, self._kargs['properties'])
            parent.append_item(item)
        elif self._kargs['link'] == 'section':
            section = Gio.Menu()
            parent.append_section(self._kargs['label'], section)
        elif self._kargs['link'] == 'submenu':
            submenu = Gio.Menu()
            parent.append_submenu(self._kargs['label'], submenu)

    def removeItem(self):
        menuId = self._kargs['menuId']
        (menu, mId) = self._list.getMenu(menuId)
        if mId != -1:
            menu.remove(mId)
        else:
            print "Remove menu item"

    def run(self):
        if self._action == 'append':
            self.appendItem()
        elif self._action == 'remove':
            self.removeItem()

class ActionList(object):
    def __init__(self, objectPath):
        self._actions = []
        self._objectPath = objectPath
        self._bux = None
        self._exportID = None
        self._ownNameID = None
        self._root = None

    def appendItem(self, label, actionName, link=None, parentId=None,  properties=None):
        self._actions.append(Action(self, 'append',
                                    parentId=parentId,
                                    label=label,
                                    actionName=actionName,
                                    link=link,
                                    properties=properties))

    def removeItem(self, menuId):
        self._actions.append(Action(self, 'remove',
                                    menuId=menuId))

    def _findMenu(self, root, ids):
        if len(ids) == 0:
            return (root, -1)

        currentId = int(ids[0])
        link = root.get_item_link(currentId, 'section')
        if link == None:
            link = root.get_item_link(currentId, 'submenu')

        if link:
            return self._findMenu(link, ids[1:])
        else:
            return (root, currentId)

    def getMenu(self, menuId):
        return self._findMenu(self._root, str(menuId).split('.'));

    def walk(self):
        item = self._actions.pop(0)
        item.run()

    def size(self):
        return len(self._actions)

    def _exportService(self, connection, name):
        self._root = Gio.Menu()
        self._bus = connection
        self._exportID = connection.export_menu_model(MENU_OBJECT_PATH, self._root)

    def start(self):
        self._ownNameID = Gio.bus_own_name(2, MENU_SERVICE_NAME, 0, self._exportService, None, None)

    def stop(self):
        if self._exportID:
            self._bus.unexport_menu_model(self._exportID)
            self._exportID = None

        if self._ownNameID:
            Gio.bus_unown_name(self._ownNameID)
            self._ownNameID = None

