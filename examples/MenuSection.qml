import QtQuick 1.1
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem


Item {
    property variant menu
    width: parent.width
    height: contents.height

    Column {
        id: contents
        width: parent.width        

        ListItem.Header {
            text: menu.label
        }

        Repeater {
            model: menu ? menu.linkSection : undefined

            MenuFactory {
                width: contents.width
                menu: model
            }
        }

        ListItem.Divider { }
    }
}
