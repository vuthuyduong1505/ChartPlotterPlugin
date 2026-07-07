import QtQuick

Rectangle {
    id: footerBar
    height: 36
    color: "#151b26"
    border.color: "#232d3d"
    border.width: 1

    property var myplotter

    Row {
        anchors.left: parent.left
        anchors.leftMargin: 16
        anchors.verticalCenter: parent.verticalCenter
        spacing: 12

        Text {
            text: "💡 Hướng dẫn:"
            color: "#64748b"; font.pixelSize: 11; font.bold: true
            anchors.verticalCenter: parent.verticalCenter
        }

        Rectangle {
            color: "#1e293b"; border.color: "#334155"; radius: 4; height: 22; width: label3.width + 12
            anchors.verticalCenter: parent.verticalCenter
            Text { id: label3; anchors.centerIn: parent; text: "⌨️ Shift/Ctrl + Cuộn: Zoom từng trục"; color: "#94a3b8"; font.pixelSize: 11 }
        }
        Rectangle {
            color: "#1e293b"; border.color: "#334155"; radius: 4; height: 22; width: label4.width + 12
            anchors.verticalCenter: parent.verticalCenter
            Text { id: label4; anchors.centerIn: parent; text: "✂️ Crop: Cắt vùng xem"; color: "#94a3b8"; font.pixelSize: 11 }
        }
    }

    Row {
        anchors.right: parent.right
        anchors.rightMargin: 16
        anchors.verticalCenter: parent.verticalCenter
        spacing: 8

        Rectangle {
            width: 8; height: 8; radius: 4
            color: myplotter && myplotter.nearestPointMap.valid ? "#10b981" : "#64748b"
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            text: {
                if (!myplotter || !myplotter.nearestPointMap.valid) return "📍 Tọa độ: ---"
                if (myplotter.nearestPointMap.isPie) {
                    return "📍 Lát cắt: " + myplotter.nearestPointMap.sliceName + " (" + myplotter.nearestPointMap.percent.toFixed(1) + "%)"
                }
                return "📍 Tọa độ: X = " + myplotter.nearestPointMap.dataX.toFixed(3) + "  |  Y = " + myplotter.nearestPointMap.dataY.toFixed(3)
            }
            color: myplotter && myplotter.nearestPointMap.valid ? "#38bdf8" : "#64748b"
            font.pixelSize: 12
            font.bold: true
            font.family: "Monospace"
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
