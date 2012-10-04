// This example QML application renders a menu model exposed on the session bus
// under the well-known name com.canonical.testmenu and at the object path
// /com/canonical/testmenu.

import QtQuick 1.1
import QMenuModel 0.1

Item {
    id: container
    width: 300
    height: 300

    QDBusMenuModel {
        id: menuModel
        busType: DBus.SessionBus
        busName: "com.canonical.testmenu"
        objectPath: "/com/canonical/testmenu"
        onStatusChanged: console.log("status of menu model changed to", status)
    }

    ListView {
        id: view
        property variant __back: []
        anchors.left: parent.left
        anchors.right:parent.right
        anchors.top: parent.top
        anchors.bottom: backbutton.top
        anchors.margins: 10
        spacing: 3
        model: menuModel
        Component.onCompleted: menuModel.start()
        delegate: Rectangle {
            width: parent.width
            height: 30
            radius: 3
            color: {
                if (linkSubMenu == null) return "lightgrey"
                if (delegatearea.containsMouse) return "steelblue"
                return "lightsteelblue"
            }
            Text {
                anchors.fill: parent
                anchors.margins: 5
                verticalAlignment: Text.AlignVCenter
                color: (linkSubMenu == null) ? "grey" : "black"
                text: {
                    if (linkSubMenu == null) return "%1 (%2)".arg(label).arg(action)
                    else return "submenu"
                }
            }
            MouseArea {
                id: delegatearea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    if (linkSubMenu == null) return
                    var newback = view.__back
                    newback.push(view.model)
                    view.__back = newback
                    view.model = linkSubMenu
                }
                onEntered: {
                    var text = ""
                    for (var prop in extra) {
                        text += "%1 = %2\n".arg(prop).arg(extra[prop].toString())
                    }
                    if (text != "") {
                        tooltip.text = text
                        tooltip.visible = true
                    }
                }
                onExited: {
                    tooltip.visible = false
                }
                onPositionChanged: {
                    var pos = container.mapFromItem(delegatearea, mouse.x, mouse.y)
                    tooltip.x = pos.x + 10
                    tooltip.y = pos.y + 10
                }
            }
        }
        function goback() {
            var newback = view.__back
            model = newback[newback.length - 1]
            newback.pop()
            view.__back = newback
        }
    }

    Rectangle {
        id: backbutton
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 5
        radius: 4
        height: 50
        color: (mousearea.enabled && mousearea.containsMouse) ? "steelblue" : "lightsteelblue"
        Text {
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: "back"
            color: mousearea.enabled ? "black" : "grey"
        }
        MouseArea {
            id: mousearea
            anchors.fill: parent
            enabled: view.__back.length > 0
            hoverEnabled: true
            onClicked: {
                view.goback()
            }
        }
    }

    Rectangle {
        id: tooltip
        property alias text: t.text
        visible: false
        width: t.width + 10
        height: t.height + 10
        radius: 4
        color: "lightyellow"
        Text {
            id: t
            height: paintedHeight
            width: paintedWidth
            x: 5
            y: 5
            anchors.margins: 4
            font.pixelSize: 12
        }
    }
}

