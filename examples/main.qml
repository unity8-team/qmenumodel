import QtQuick 1.1
import QMenuModel 1.0
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem

Rectangle {
    id: main

    height: 800
    width: 480
    color: "#eeeeee"

    function loadMainMenu(menu) {
        mainMenu.model = menu
    }

    QDBusMenuModel {
        id: menuModel
        busType: 1
        busName: "com.ubuntu.networksettings"
        objectPath: "/com/ubuntu/networksettings"
        onConnected: {
            console.log("Menu appears ")
        }
    }


    ListView {
        id: mainMenu
        anchors.fill: parent
        model: menuModel

        delegate: MenuFactory {
            width: parent.width
            menu: model
        }

        Component.onCompleted: {
            menuModel.connect()
        }
    }
}


