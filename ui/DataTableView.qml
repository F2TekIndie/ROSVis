import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import ROSVis



Rectangle {
    id: root
    required property UIAdapter adapter
    required property TopicDataModel dataModel
    property string requestError: ""

    color: "#2A3540"
    border.color: "#4A5B6B"
    radius: 10

    Connections {
        target: root.adapter

        function onCurrentTopicChanged(topic) {
            root.requestError = ""
        }
        function onLatestReceived(topic, data) {
            if (topic === root.dataModel.currentTopic)
                root.requestError = ""
        }
        function onErrorOccurred(message) {
            root.requestError = message
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        RowLayout {
            Layout.fillWidth: true

            Label {
                Layout.fillWidth: true
                text: root.dataModel.currentTopic || qsTr("No topic selected")
                color: "#FFFFFF"
                font.bold: true
                elide: Text.ElideMiddle
            }
            Button {
                text: qsTr("Refresh")
                enabled: root.dataModel.currentTopic.length > 0
                onClicked: {
                    root.requestError = ""
                    root.adapter.getLatest()
                }
            }
        }

        Label {
            Layout.fillWidth: true
            visible: root.dataModel.messageType.length > 0
            text: root.dataModel.messageType
            color: "#ADC1D1"
            elide: Text.ElideRight
        }

        Label {
            Layout.fillWidth: true
            visible: root.dataModel.hasData
            text: qsTr("Sample %1 · Received %2").arg(root.dataModel.sequence)
                .arg(root.dataModel.receivedTime.toLocaleString(Qt.locale(), "yyyy-MM-dd HH:mm:ss.zzz"))
            color: "#ADC1D1"
            font.pixelSize: 12
        }

        Label {
            Layout.fillWidth: true
            visible: text.length > 0
            text: root.requestError || root.dataModel.conversionError
            color: "#FFB4AB"
            wrapMode: Text.Wrap
        }

        Label {
            Layout.fillWidth: true
            visible: !root.dataModel.hasData
            text: root.dataModel.currentTopic.length === 0
                ? qsTr("Select a topic to inspect its latest sample.")
                : qsTr("Waiting for the first message. Subscribe to the topic, then refresh.")
            color: "#ADC1D1"
            wrapMode: Text.Wrap
        }

        RowLayout {
            Layout.fillWidth: true
            visible: root.dataModel.hasData

            Label {
                Layout.preferredWidth: Math.max(100, dataView.width * 0.55)
                text: qsTr("Field")
                color: "#ADC1D1"
                font.bold: true
            }
            Label {
                Layout.fillWidth: true
                text: qsTr("Value")
                color: "#ADC1D1"
                font.bold: true
            }
        }

        ListView {
            id: dataView
            model: root.dataModel
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 2

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            delegate: Rectangle {
                id: row
                required property int index
                required property string fieldPath
                required property string displayValue
                required property string valueKind

                width: dataView.width
                height: Math.max(28, valueLabel.implicitHeight + 8)
                color: index % 2 === 0 ? "#26313C" : "transparent"
                radius: 3

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 4
                    spacing: 8

                    Label {
                        Layout.preferredWidth: Math.max(100, dataView.width * 0.55) - 4
                        Layout.alignment: Qt.AlignTop
                        text: row.fieldPath
                        color: "#DCE4EB"
                        elide: Text.ElideMiddle
                        ToolTip.visible: pathHover.hovered
                        ToolTip.text: row.fieldPath
                        HoverHandler { id: pathHover }
                    }
                    Label {
                        id: valueLabel
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignTop
                        text: row.displayValue
                        color: row.valueKind === "summary" ? "#FFCF8A" : "#FFFFFF"
                        wrapMode: Text.WrapAnywhere
                    }
                }
            }

            Label {
                anchors.centerIn: parent
                visible: root.dataModel.hasData && dataView.count === 0
                text: qsTr("The received message has no fields.")
                color: "#ADC1D1"
            }
        }
    }
}
