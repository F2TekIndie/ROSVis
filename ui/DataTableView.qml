import QtQuick
import QtQuick.Layouts
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
            if ( activeTopic.text !== topic )
                dataModel.clear()
            activeTopic.text = "Active topic: " + topic
        }

        function onLatestReceived( topic, data ){
            dataModel.clear()
            console.log( "Look at the data: " + data.count );
        }
    }

    ListModel{
        id: dataModel
    }
    //
    ColumnLayout{
        anchors.fill: parent
        anchors.margins: 8

        Label{
            id: activeTopic
            Layout.fillWidth: true
            Layout.maximumHeight: 30

            color: "red"
            font.bold: true
            text: "No topic selected"
        }
        ListView{
            id: dataView
            model: dataModel

            Layout.fillWidth: true
            Layout.fillHeight: true
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
}
