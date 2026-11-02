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

        delegate: RosTopicsViewItem {
            onClicked: {
                topicsList.currentIndex = index
                const topic = topicsModel.get( index ).topicName
                root.adapter.currentTopic = topic
            }

            onSubscriptionToggled: function(topic, subscribed){
                root.adapter.currentTopic = topic
                root.adapter.sendCommand( subscribed ? Commands.RC_SUBSCRIBE : Commands.RC_UNSUBSCRIBE )
            }
        }
    }
}
