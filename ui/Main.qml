import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import ROSVis


Item {
    id: window
    width: 800
    height: 6000
    visible: true

    property bool lightMode: Application.styleHints.colorScheme === Qt.Dark
    property color reallyDark: "#1f1f1f"
    property color dark: "#262626"
    property color reallyLight: "#e7e7e7"
    property color light: "#e0e0e0"

    ColumnLayout {
        id: main
        anchors.fill: parent

        ColumnLayout {
            id: data

            VulkanView {
                id: renderer
            }

            DataTableView {
                id: dataTable
            }
        }

        RosTopicsView{
            id: topics
        }
    }

}
