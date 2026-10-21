import QtQuick



Rectangle {
    id: root
    color: "red"
    radius: 10

    function addTopic(topic){
        topicsModel.append( {topicName: topic} );
    }

    ListModel{
        id: topicsModel
    }

    ListView{
        anchors.fill: parent
        clip: true
        model: topicsModel

        delegate: Text{
            required property string topicName
            text: topicName
            color: "white"
        }
    }
}
