import QtQuick
import QtQuick.Controls
import QtQuick.Layouts



ItemDelegate {
    id: row

    required property string schemaName
    required property int index

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
                id: schemaLabel
                text: row.schemaName
                color: row.highlighted ? "#FFFFFF" : "#DCE4EB"
                font.pixelSize: 14
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideMiddle
            }
        }
    }

    leftPadding: 12
    rightPadding: 12
}
