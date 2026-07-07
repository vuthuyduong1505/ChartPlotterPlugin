import QtQuick

Rectangle {
    id: tooltip
    color: "#f2181e29"
    border.color: "#475569"; border.width: 1; radius: 8
    visible: hoverHandler && hoverHandler.hovered && myplotter && myplotter.nearestPointMap.valid
    z: 10
    width: tooltipLayout.width + 20; height: tooltipLayout.height + 16

    property var myplotter
    property var hoverHandler

    x: {
        if (!parent || !myplotter) return 0;
        var mx = myplotter.nearestPointMap.valid ? myplotter.nearestPointMap.screenX : 0;
        var targetX = mx + 14;
        if (targetX + width > parent.width) targetX = mx - width - 14;
        return Math.max(4, Math.min(parent.width - width - 4, targetX));
    }
    y: {
        if (!parent || !myplotter) return 0;
        var my = myplotter.nearestPointMap.valid ? myplotter.nearestPointMap.screenY : 0;
        var targetY = my - height - 14;
        if (targetY < 4) targetY = my + 14;
        return Math.max(4, Math.min(parent.height - height - 4, targetY));
    }

    Column {
        id: tooltipLayout
        anchors.centerIn: parent
        spacing: 4

        Text {
            color: "#ffffff"; font.pixelSize: 12; font.bold: true; font.family: "Monospace"
            text: myplotter && myplotter.nearestPointMap.valid 
                  ? (myplotter.nearestPointMap.isPie 
                     ? "Tỷ lệ: " + (myplotter.nearestPointMap.percent < 0.01 && myplotter.nearestPointMap.percent > 0 ? "<0.01" : myplotter.nearestPointMap.percent.toFixed(1)) + "% (" + myplotter.nearestPointMap.dataY + " điểm)"
                     : "X: " + myplotter.nearestPointMap.dataX.toFixed(3)) 
                  : ""
        }

        Text {
            color: "#f1c40f"; font.pixelSize: 12; font.bold: true; font.family: "Monospace"
            visible: myplotter && myplotter.nearestPointMap.valid && !myplotter.nearestPointMap.isPie
            text: myplotter && myplotter.nearestPointMap.valid && !myplotter.nearestPointMap.isPie 
                  ? "Y: " + myplotter.nearestPointMap.dataY.toFixed(3) : ""
        }

        Text {
            color: "#34d399"; font.pixelSize: 11; font.bold: true; font.family: "Monospace"
            visible: myplotter && myplotter.nearestPointMap.valid && myplotter.nearestPointMap.isPie
            text: {
                if (!myplotter || !myplotter.nearestPointMap.valid || !myplotter.nearestPointMap.isPie) return "";
                if (myplotter.nearestPointMap.sliceName === "Ngoại lai (Outliers)") return "Khoảng giá trị: Khác";
                var minVal = myplotter.nearestPointMap.currentBinMin;
                var maxVal = myplotter.nearestPointMap.currentBinMax;
                return "Khoảng giá trị: " + (minVal !== undefined ? minVal.toFixed(3) : "0.000") + " -> " + (maxVal !== undefined ? maxVal.toFixed(3) : "0.000");
            }
        }
    }
}
