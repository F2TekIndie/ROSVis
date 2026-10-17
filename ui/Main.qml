import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import ROSVis


Item {
    id: root
    visible: true
    Rectangle{
        width: 1600
        height: 1000
        color: "green"

        RowLayout {
            id: main
            anchors.fill: parent

            ColumnLayout {
                id: data

                VulkanView {
                    id: renderer
                    width: 1000
                    height: 600
                }

                DataTableView {
                    id: dataTable
                    width: 1000
                    height: 400
                }
            }

            RosTopicsView{
                id: topics
                width: 600
                height: 1000
            }
        }
    }
}
