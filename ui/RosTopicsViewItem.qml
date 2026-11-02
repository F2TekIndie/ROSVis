import QtQuick
import QtQuick.Controls
import QtQuick.Layouts



ItemDelegate {
    id: row

    required property string topicName
    required property int index

    signal subscriptionToggled(string topic, bool subscribed)

    width: ListView.view.width
    height: 44
    hoverEnabled: true

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

    highlighted: ListView.isCurrentItem

    contentItem: Item{
        RowLayout{
            Label {
                id: topicLabel
                text: row.topicName
                color: row.highlighted ? "#FFFFFF" : "#DCE4EB"
                font.pixelSize: 14
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideMiddle
            }
            CheckBox {
                id: topicsub
                text: qsTr("Subscribed")
                checked: false

                onToggled: row.subscriptionToggled( row.topicName, checked )

                contentItem: Text {
                    text: topicsub.text
                    color: row.highlighted ? "#FFFFFF" : "#DCE4EB"
                    font.pixelSize: 14
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: topicsub.indicator ? topicsub.indicator.width + topicsub.spacing : 0
                }
            }
        }
    }

    leftPadding: 12
    rightPadding: 12

    ToolTip.visible: hovered && topicLabelTruncated
    ToolTip.text: topicName
    ToolTip.delay: 700

    readonly property bool topicLabelTruncated:
        topicLabel.truncated
}
