import QtQuick 2.0
import QMenuModel 0.1

Item {
    width: 100
    height: 100

    QDBusMenuModel {
        id: menuModel
        busType: globalBusType
        busName: globalBusName
        objectPath: globalObjectPath
    }

    ListView {
        model: menuModel
        delegate: Item {}
    }

    Component.onCompleted: menuModel.start()
}

