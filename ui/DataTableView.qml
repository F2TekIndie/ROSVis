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


    Connections {
        target: root.adapter

        function onCurrentTopicChanged(topic) {
            dataModel.clear()
            dataModel.append( {name: "Current topic to deliver the data: " + topic} )
        }
    }

    ListModel{
        id: dataModel
    }
    ListView{
        id: dataView
        model: dataModel

        anchors.fill: parent
        anchors.margins: 8
        clip: true
        spacing: 4
        currentIndex: -1

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
        }

        delegate: Label{
            color: "red"
            text: name
        }
    }
}
