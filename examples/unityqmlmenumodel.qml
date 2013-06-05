
import QtQuick 2.0
import QMenuModel 0.1

Item {
    width: 400;
    height: 500;

    UnityMenuModel {
        id: menu
        busName: "com.canonical.testmenu"
        actionObjectPath: "/com/canonical/testmenu"
        menuObjectPath: "/com/canonical/testmenu"
    }

    ListView {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 3
        model: menu

        delegate: Loader {
            sourceComponent: isSeparator ? separator : menuitem;

            Component {
                id: separator
                Rectangle {
                    width: parent.width
                    height: 4
                    color: "blue"
                }
            }

            Component {
                id: menuitem
                Rectangle {
                    width: parent.width
                    height: 40
                    color: "#ddd"
                    Text {
                        anchors.fill: parent
                        anchors.margins: 5
                        verticalAlignment: Text.AlignVCenter
                        color: sensitive ? "black" : "#aaa";
                        text: label
                    }
                }
            }
        }
    }
}
