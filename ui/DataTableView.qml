import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import ROSVis
import Robotics.Commands 1.0




Rectangle {
    id: root
    required property UIAdapter adapter
    required property TopicDataModel dataModel

    color: "#2A3540"
    border.color: "#4A5B6B"
    radius: 10


    Connections {
        target: root.adapter

        function onCurrentTopicChanged(topic) {
            activeTopic.text = "Active topic: " + topic
        }
    }

    ColumnLayout{
        anchors.fill: parent
        anchors.margins: 8

        RowLayout{
            Layout.maximumHeight: 30

            Label{
                id: activeTopic
                Layout.fillWidth: true
                Layout.maximumHeight: 30

                color: "red"
                font.bold: true
                text: "No topic selected"
            }
            Button{
                id: refreshData
                text: "Refresh"

                onClicked: {
                    root.adapter.getLatest()
                }
            }
        }

        ListView{
            id: dataView
            model: root.dataModel

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
                text: "Type of data " + type
            }
        }
    }
}
