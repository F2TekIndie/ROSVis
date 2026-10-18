import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import ROSVis


ApplicationWindow {
    id: root
    visible: true

    menuBar: MenuBar{

    }
    header: ToolBar{

    }
    footer: TabBar{

    }

    RowLayout {
        id: main
        anchors.fill: parent

        ColumnLayout {
            id: data

            VulkanView {
                id: renderer
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: 1000
                Layout.minimumHeight: 600
            }

            DataTableView {
                id: dataTable
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: 1000
                Layout.minimumHeight: 400
            }
        }

        RosTopicsView{
            id: topics
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: 600
            Layout.minimumHeight: 1000
            Layout.maximumWidth: 1000
        }
    }
}
