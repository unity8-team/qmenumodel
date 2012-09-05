import QtQuick 1.1
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem


ListItem.Standard {
    property variant menu

    width: parent.width
    text: menu.label

    onClicked: {
        if (menu.linkSubMenu)
            loadMainMenu(menu.linkSubMenu)
        else
            loadMainMenu(menu.linkSection)
    }
}
