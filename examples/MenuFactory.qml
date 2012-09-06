import QtQuick 1.1
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem

Loader {
    property variant menu
    //height: childrenRect.height

    onMenuChanged: {
        if (menu) {
            if (menu.linkSection)
                source = "MenuSection.qml"
            else if (menu.linkSubMenu)
                source = "SubMenu.qml"
            else
                source = "Menu.qml";
        }

        console.debug("Load: " + source)
    }

    onStatusChanged: {
        if (status == Loader.Ready) {
            if (menu.linkSection)
                item.menu = menu
            else if (menu.linkSubMenu)
                item.menu = menu
            else if (menu)
                item.menu = menu
        }
    }
}

