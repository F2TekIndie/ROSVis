import QtQuick
import QtQuick.Controls
import ROSVis
import Robotics.Commands 1.0



Rectangle {
    id: root

    required property UIAdapter adapter

    color: "#2A3540"
    border.color: "#4A5B6B"
    radius: 10

    onVisibleChanged: {
        if (visible) {
            schemaModel.clear()
            adapter.getSchemas()
        }
    }

    Connections {
        target: root.adapter

        function onSchemas( schemas ) {
            schemaModel.clear()
            console.log( schemas.count )
            for ( const schema of schemas )
                schemaModel.append( {"schemaName": schema} )
        }
        function onErrorOccurred(message) {
            console.warn("ROSAdapter: " + message)
        }
    }

    ListModel{
        id: schemaModel
    }

    ListView {
        id: schemaList
        anchors.fill: parent
        anchors.margins: 8
        clip: true
        spacing: 4

        model: schemaModel
        currentIndex: -1

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
        }

        delegate: RosSchemaViewItem {
        }
    }
}
