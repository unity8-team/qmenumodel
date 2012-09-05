import QtQuick 1.1
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem


ListItem.Standard {
    property variant menu

    anchors.fill: parent
    text: menu.label
    progression: true

    onClicked: { loadMainMenu (menu.linkSubMenu) }
}
