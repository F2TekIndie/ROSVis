import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import ROSVis


ApplicationWindow {
    id: root
    visible: true
    required property UIAdapter rossy

    menuBar: MenuBar{
        id: menu
    }
    header: ToolBar{
        id: header
    }
    footer: TabBar{
        id: footer
    }

    RowLayout {
        id: main
        anchors.fill: parent

        ColumnLayout {
            id: dataLayout

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

        ColumnLayout{
            id: topicsLayout
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.maximumWidth: 1000

            TabBar{
                id: topicsBar
                Layout.fillWidth: true
                Layout.minimumHeight: 30
                Layout.maximumHeight: 30
                TabButton{
                    text: qsTr("Topics")
                }
                TabButton{
                    text: qsTr("More Topics")
                }
            }

            StackLayout{
                id: detailLayout
                Layout.minimumWidth: 600
                Layout.minimumHeight: 1000
                currentIndex: topicsBar.currentIndex

                RosTopicsView{
                    id: topicsTab
                    onVisibleChanged: {
                        rossy.sendCommand( UIAdapter.RC_GET_TOPICS )
                    }
                }

                Rectangle{
                    id: moretopicsTab
                    color: "green"
                }
            }
        }
    }
}
