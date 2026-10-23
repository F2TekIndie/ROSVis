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
            topicsModel.clear()
            adapter.sendCommand( Commands.RC_GET_TOPICS )
        }
    }

    function addTopic(topic){
        topicsModel.append( {topicName: topic} );
    }

    Connections {
        target: root.adapter

        function onNewTopic(topic) {
            root.addTopic(topic)
        }
    }

    ListModel{
        id: topicsModel
    }

    ListView {
        id: topicsList
        anchors.fill: parent
        anchors.margins: 8
        clip: true
        spacing: 4

        model: topicsModel
        currentIndex: -1

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
        }

        delegate: ItemDelegate {
            id: row

            required property string topicName
            required property int index

            width: ListView.view.width
            height: 44
            hoverEnabled: true

            onClicked: topicsList.currentIndex = index

            background: Rectangle {
                radius: 6

                color: row.highlighted ? "#42596B"
                     : row.hovered     ? "#354350"
                                       : "transparent"

                border.width: row.highlighted ? 1 : 0
                border.color: "#7FA6C2"

                Behavior on color {
                    ColorAnimation { duration: 120 }
                }
            }

            highlighted: topicsList.currentIndex === index

            contentItem: Label {
                text: row.topicName
                color: row.highlighted ? "#FFFFFF" : "#DCE4EB"
                font.pixelSize: 14
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideMiddle
            }

            leftPadding: 12
            rightPadding: 12

            ToolTip.visible: hovered && topicLabelTruncated
            ToolTip.text: topicName
            ToolTip.delay: 700

            readonly property bool topicLabelTruncated:
                contentItem.truncated
        }
    }
}
