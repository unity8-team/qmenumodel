import QtQuick 2.0
import QMenuModel 0.1

Item {
    id: root
    width: 100
    height: 100

    property bool reset: resetModel

    onResetChanged: {
        if (reset) {
            // destroy the current model and check if it will not crash the QML engine
            view.model.destroy();
        }
    }

    ListView {
        id: view
        anchors.fill: parent
        delegate: Text {
            text: label
        }
    }

    Component.onCompleted: {
        // dynamically create the model to destroy it later
        var model = Qt.createQmlObject("import QMenuModel 0.1; QDBusMenuModel { id: menuModel; busType: globalBusType; busName: globalBusName; objectPath: globalObjectPath; }", view, "");
        model.start();
        view.model = model;
    }
}

