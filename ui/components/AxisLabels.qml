import QtQuick

Item {
    id: root
    anchors.fill: parent

    property var myplotter

    // Nhãn trục X dưới lề đồ thị
    Repeater {
        model: myplotter && myplotter.chartType !== 2 ? myplotter.xTicks : null
        delegate: Text {
            text: modelData.value; color: "#94a3b8"; font.pixelSize: 10; font.family: "Monospace"
            x: myplotter ? myplotter.x + myplotter.dataToX(modelData.val) - width / 2 : 0
            y: myplotter ? myplotter.y + myplotter.height + 6 : 0
            visible: myplotter && x >= myplotter.x - 2 && x + width <= myplotter.x + myplotter.width + 2
        }
    }

    // Nhãn trục Y bên lề trái đồ thị
    Repeater {
        model: myplotter && myplotter.chartType !== 2 ? myplotter.yTicks : null
        delegate: Text {
            text: modelData.value; color: "#94a3b8"; font.pixelSize: 10; font.family: "Monospace"
            x: myplotter ? myplotter.x - width - 10 : 0
            y: myplotter ? myplotter.y + myplotter.dataToY(modelData.val) - height / 2 : 0
            visible: myplotter && y >= myplotter.y - 2 && y + height <= myplotter.y + myplotter.height + 2
        }
    }
}
