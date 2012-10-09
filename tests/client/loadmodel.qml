import QtQuick 2.0
import QMenuModel 0.1

Item {
    id: root
    width: 100
    height: 100

    property bool reset: resetModel

    onResetChanged: {
        if (reset) {
            console.log("Remove page");
            view.model.destroy();
            //pop();
        }
    }

    ListView {
        id: view
        anchors.fill: parent
        delegate: Text {
            text: label
        }
        onCountChanged: {
            console.log("Row count: " + count);
        }
    }

    Component.onCompleted: {
        var model = Qt.createQmlObject("import QMenuModel 0.1; QDBusMenuModel { id: menuModel; busType: globalBusType; busName: globalBusName; objectPath: globalObjectPath; }", view, "");
        model.start();
        console.log("New model: " + model)
        console.log("New model2: " + model)
        view.model = model;
    }
}

