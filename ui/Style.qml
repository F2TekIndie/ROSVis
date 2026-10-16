
pragma Singleton
import QtQuick 2.0

QtObject {
    property QtObject sys: QtObject{
        property int width: 1000
        property int height: 1000
        property color background: "white"
    }

    property QtObject menu: QtObject{
        property int height: 25
        property QtObject button: QtObject{
            property int width: 80
            property int height: 25
        }
    }

    property QtObject border: QtObject {
        property int width: 1
        property QtObject color: QtObject{
            property color active: "red"
            property color inactive: "blue"
        }
    }

    property QtObject text: QtObject{
        property int size: 20
        property color color: "green"
    }

    property QtObject devices: QtObject{
        property color listbackground: "red"
        property color mapbackground: "green"
        property color propertiesbackground: "blue"
    }
}