import QtQuick
import QtQuick.Controls 2.15

Item {
    id: root
    anchors.fill: parent

    property var myplotter
    property var hoverHandler

    // Chấm tròn tiêu điểm tại điểm bắt dính
    Rectangle {
        id: snapDot
        width: 12; height: 12; radius: 6
        color: "#f1c40f"; border.color: "#ffffff"; border.width: 1.5
        x: myplotter && myplotter.nearestPointMap.valid ? myplotter.dataToX(myplotter.nearestPointMap.dataX) - width / 2 : 0
        y: myplotter && myplotter.nearestPointMap.valid ? myplotter.dataToY(myplotter.nearestPointMap.dataY) - height / 2 : 0
        visible: hoverHandler && hoverHandler.hovered && myplotter && myplotter.nearestPointMap.valid && !myplotter.nearestPointMap.isPie
        z: 8
    }

    // Crosshair dọc
    Rectangle {
        id: crosshairVertical
        width: 1; height: parent.height; color: "#40ffffff"
        x: myplotter && myplotter.nearestPointMap.valid ? myplotter.nearestPointMap.screenX : 0
        visible: hoverHandler && hoverHandler.hovered && myplotter && myplotter.nearestPointMap.valid && !myplotter.nearestPointMap.isPie
        z: 5
    }

    // Crosshair ngang
    Rectangle {
        id: crosshairHorizontal
        width: parent.width; height: 1; color: "#40ffffff"
        y: myplotter && myplotter.nearestPointMap.valid ? myplotter.nearestPointMap.screenY : 0
        visible: hoverHandler && hoverHandler.hovered && myplotter && myplotter.nearestPointMap.valid && !myplotter.nearestPointMap.isPie
        z: 5
    }

    // Rubber Band selection rectangle
    Rectangle {
        id: cropRect
        visible: false
        color: "#303b82f6"; border.color: "#3b82f6"; border.width: 1.5
        z: 15
    }

    // Intercept mouse clicks and drags in Crop Mode
    MouseArea {
        anchors.fill: parent
        enabled: myplotter && myplotter.isCropMode
        cursorShape: enabled ? Qt.CrossCursor : Qt.ArrowCursor
        z: 14
        property real startX: 0
        property real startY: 0

        onPressed: (mouse) => {
            startX = mouse.x; startY = mouse.y
            cropRect.x = startX; cropRect.y = startY
            cropRect.width = 0; cropRect.height = 0; cropRect.visible = true
        }

        onPositionChanged: (mouse) => {
            if (!myplotter) return
            var curX = Math.max(0, Math.min(myplotter.width, mouse.x))
            var curY = Math.max(0, Math.min(myplotter.height, mouse.y))
            cropRect.x = Math.min(startX, curX); cropRect.y = Math.min(startY, curY)
            cropRect.width = Math.abs(curX - startX); cropRect.height = Math.abs(curY - startY)
        }

        onReleased: (mouse) => {
            cropRect.visible = false
            if (!myplotter) return
            var selW = cropRect.width; var selH = cropRect.height
            if (selW > 5 && selH > 5) {
                var newZoomX = myplotter.zoomX * (myplotter.width / selW)
                var newZoomY = myplotter.zoomY * (myplotter.height / selH)
                var dataRangeX = myplotter.dataMaxX - myplotter.dataMinX
                var dataRangeY = myplotter.dataMaxY - myplotter.dataMinY
                var viewRangeX = dataRangeX / myplotter.zoomX
                var viewRangeY = dataRangeY / myplotter.zoomY
                var newPanX = myplotter.panX + (cropRect.x / myplotter.width) * viewRangeX
                var newPanY = myplotter.panY + ((myplotter.height - (cropRect.y + cropRect.height)) / myplotter.height) * viewRangeY
                myplotter.zoomX = newZoomX; myplotter.zoomY = newZoomY
                myplotter.panX = newPanX; myplotter.panY = newPanY
            }
            myplotter.isCropMode = false
        }
    }

    // Horizontal ScrollBar
    ScrollBar {
        id: hScrollBar
        orientation: Qt.Horizontal
        anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.bottom
        anchors.topMargin: 22
        z: 20
        visible: myplotter && myplotter.chartType !== 2 && (myplotter.dataMaxX > myplotter.dataMinX)
        size: myplotter && (myplotter.zoomX > 0.0) ? (1.0 / myplotter.zoomX) : 1.0
        position: {
            if (!myplotter) return 0.0
            var dataRangeX = myplotter.dataMaxX - myplotter.dataMinX
            if (dataRangeX <= 0.0) return 0.0
            return Math.max(0.0, Math.min(1.0 - size, (myplotter.panX - myplotter.dataMinX) / dataRangeX))
        }
        onPositionChanged: {
            if (!myplotter) return
            if (activeFocus || pressed) {
                var dataRangeX = myplotter.dataMaxX - myplotter.dataMinX
                var newPanX = myplotter.dataMinX + position * dataRangeX
                if (Math.abs(myplotter.panX - newPanX) > 1e-5) myplotter.panX = newPanX
            }
        }
    }
}
