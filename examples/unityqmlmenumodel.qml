
import QtQuick 2.0
import QMenuModel 0.1

Item {
    width: 400;
    height: 500;

    UnityMenuModel {
        id: menu
        busName: "com.canonical.indicator.sound"
        actions: { "indicator": "/com/canonical/indicator/sound" }
        menuObjectPath: "/com/canonical/indicator/sound/desktop"
    }

    ListView {
        id: listview
        anchors.fill: parent
        anchors.margins: 10
        spacing: 3
        model: menu

        delegate: Loader {
            sourceComponent: {
                if (isSeparator) {
                    return separator;
                }
                else if (type == "com.canonical.unity.slider") {
                    listview.model.loadExtendedAttributes(index, {'min-icon': 'icon',
                                                                  'max-icon': 'icon'});
                    return slider;
                }
                else {
                    return menuitem;
                }
            }

            Component {
                id: separator
                Rectangle {
                    width: listview.width
                    height: 4
                    color: "blue"
                }
            }

            Component {
                id: slider
                Row {
                    anchors.fill: parent
                    Image {
                        source: ext.minIcon
                    }
                    Image {
                        source: ext.maxIcon
                    }
                }
            }

            Component {
                id: menuitem
                Rectangle {
                    width: listview.width
                    height: 40
                    color: "#ddd"
                    Row {
                        anchors.fill: parent
                        anchors.margins: 5
                        Image {
                            source: icon
                        }
                        Text {
                            height: parent.height
                            verticalAlignment: Text.AlignVCenter
                            color: sensitive ? "black" : "#aaa";
                            text: label
                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            var submenu = listview.model.submenu(index);
                            if (submenu)
                                listview.model = submenu;
                            else
                                listview.model.activate(index);
                        }
                    }
                }
            }
        }
    }
}
