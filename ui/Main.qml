import QtQuick
import MyChartLibrary 1.0
import QtQuick.Controls 2.15
import QtQuick.Dialogs

Window {
    width: 800
    height: 520
    visible: true
    title: qsTr("Test QML")

    MyChart {
        id: myplotter
        anchors.fill: parent
        anchors.margins: 40
        chartType: typeSelector.currentIndex
        dataMode: modeSelector.currentIndex
        lineStyle: lineStyleSelector.currentIndex
        chartColor: "#f1c40f"
        Behavior on chartColor {
            ColorAnimation { duration: 150 }
        }

        property bool isCropMode: false

        // Hàm ánh xạ chung từ trị số dữ liệu sang tọa độ pixel trên màn hình (local)
        function dataToX(val) {
            var rangeX = myplotter.viewMaxX - myplotter.viewMinX;
            if (rangeX <= 0.0) return 0.0;
            return ((val - myplotter.viewMinX) / rangeX) * myplotter.width;
        }

        function dataToY(val) {
            var rangeY = myplotter.viewMaxY - myplotter.viewMinY;
            if (rangeY <= 0.0) return 0.0;
            return (1.0 - (val - myplotter.viewMinY) / rangeY) * myplotter.height;
        }

        // Bản đồ dữ liệu của điểm gần nhất
        readonly property var nearestPointMap: myplotter.getNearestDataPoint(
            hoverHandler.point.position.x,
            hoverHandler.point.position.y,
            myplotter.width,
            myplotter.height
        )

        // Bộ đón nhận sự kiện di chuột (không chặn sự kiện click/drag/zoom của biểu đồ)
        HoverHandler {
            id: hoverHandler
        }

        // Chấm tròn tiêu điểm (vàng rực rỡ, viền trắng) tại điểm bắt dính
        Rectangle {
            id: snapDot
            width: 12
            height: 12
            radius: 6
            color: "#f1c40f"
            border.color: "#ffffff"
            border.width: 1.5
            x: myplotter.nearestPointMap.valid ? myplotter.dataToX(myplotter.nearestPointMap.dataX) - width / 2 : 0
            y: myplotter.nearestPointMap.valid ? myplotter.dataToY(myplotter.nearestPointMap.dataY) - height / 2 : 0
            visible: hoverHandler.hovered && myplotter.nearestPointMap.valid
            z: 8
        }

        // Crosshair dọc
        Rectangle {
            id: crosshairVertical
            width: 1
            height: parent.height
            color: "#60ffffff"
            x: myplotter.nearestPointMap.valid ? myplotter.nearestPointMap.screenX : 0
            visible: false
            z: 5
        }

        // Crosshair ngang
        Rectangle {
            id: crosshairHorizontal
            width: parent.width
            height: 1
            color: "#60ffffff"
            y: myplotter.nearestPointMap.valid ? myplotter.nearestPointMap.screenY : 0
            visible: false
            z: 5
        }

        // Tooltip hiển thị tọa độ
        Rectangle {
            id: tooltip
            color: "#e61e272c"
            border.color: "#40ffffff"
            border.width: 1
            radius: 6
            visible: hoverHandler.hovered && myplotter.nearestPointMap.valid
            z: 10

            width: tooltipLayout.width + 16
            height: tooltipLayout.height + 12

            // Căn lề thông minh tránh bị khuất ở các cạnh biên
            x: {
                var mx = myplotter.nearestPointMap.valid ? myplotter.nearestPointMap.screenX : 0;
                var targetX = mx + 12;
                if (targetX + width > parent.width) {
                    targetX = mx - width - 12;
                }
                return Math.max(4, Math.min(parent.width - width - 4, targetX));
            }
            y: {
                var my = myplotter.nearestPointMap.valid ? myplotter.nearestPointMap.screenY : 0;
                var targetY = my - height - 12;
                if (targetY < 4) {
                    targetY = my + 12;
                }
                return Math.max(4, Math.min(parent.height - height - 4, targetY));
            }

            Column {
                id: tooltipLayout
                anchors.centerIn: parent
                spacing: 4

                Text {
                    color: "#ffffff"
                    font.pixelSize: 11
                    font.bold: true
                    font.family: "Monospace"
                    // Nếu là Pie Chart thì in ra %, ngược lại in ra X
                    text: myplotter.nearestPointMap.valid 
                          ? (myplotter.nearestPointMap.isPie 
                             ? myplotter.nearestPointMap.percent.toFixed(1) + "%" 
                             : "X: " + myplotter.nearestPointMap.dataX.toFixed(3)) 
                          : ""
                }

                Text {
                    color: "#f1c40f"
                    font.pixelSize: 11
                    font.bold: true
                    font.family: "Monospace"
                    // Ẩn hoàn toàn dòng Y nếu đang ở chế độ Pie Chart
                    visible: myplotter.nearestPointMap.valid && !myplotter.nearestPointMap.isPie
                    text: myplotter.nearestPointMap.valid && !myplotter.nearestPointMap.isPie 
                          ? "Y: " + myplotter.nearestPointMap.dataY.toFixed(3) 
                          : ""
                }
            }
        }

        // Rubber Band selection rectangle
        Rectangle {
            id: cropRect
            visible: false
            color: "#300096ff"
            border.color: "#0096ff"
            border.width: 1
            z: 15
        }

        // Intercept mouse clicks and drags in Crop Mode
        MouseArea {
            anchors.fill: parent
            enabled: myplotter.isCropMode
            cursorShape: enabled ? Qt.CrossCursor : Qt.ArrowCursor
            z: 14

            property real startX: 0
            property real startY: 0

            onPressed: (mouse) => {
                startX = mouse.x
                startY = mouse.y
                cropRect.x = startX
                cropRect.y = startY
                cropRect.width = 0
                cropRect.height = 0
                cropRect.visible = true
            }

            onPositionChanged: (mouse) => {
                var curX = Math.max(0, Math.min(myplotter.width, mouse.x))
                var curY = Math.max(0, Math.min(myplotter.height, mouse.y))

                cropRect.x = Math.min(startX, curX)
                cropRect.y = Math.min(startY, curY)
                cropRect.width = Math.abs(curX - startX)
                cropRect.height = Math.abs(curY - startY)
            }

            onReleased: (mouse) => {
                cropRect.visible = false
                var selW = cropRect.width
                var selH = cropRect.height

                if (selW > 5 && selH > 5) {
                    var newZoomX = myplotter.zoomX * (myplotter.width / selW)
                    var newZoomY = myplotter.zoomY * (myplotter.height / selH)

                    var dataRangeX = myplotter.dataMaxX - myplotter.dataMinX
                    var dataRangeY = myplotter.dataMaxY - myplotter.dataMinY
                    var viewRangeX = dataRangeX / myplotter.zoomX
                    var viewRangeY = dataRangeY / myplotter.zoomY

                    var newPanX = myplotter.panX + (cropRect.x / myplotter.width) * viewRangeX
                    var newPanY = myplotter.panY + ((myplotter.height - (cropRect.y + cropRect.height)) / myplotter.height) * viewRangeY

                    myplotter.zoomX = newZoomX
                    myplotter.zoomY = newZoomY
                    myplotter.panX = newPanX
                    myplotter.panY = newPanY
                }

                myplotter.isCropMode = false
            }
        }

        // Horizontal ScrollBar for panning navigation
        ScrollBar {
            id: hScrollBar
            orientation: Qt.Horizontal
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.bottomMargin: -25 // Đặt trong khoảng lề 40px của đồ thị
            z: 20
            visible: myplotter.chartType !== 2 && (myplotter.dataMaxX > myplotter.dataMinX)

            // Kích thước tay cầm cuộn: tỉ lệ vùng hiển thị trên tổng dữ liệu (1.0 / zoomX)
            size: (myplotter.zoomX > 0.0) ? (1.0 / myplotter.zoomX) : 1.0

            // Vị trí cuộn hiện tại
            position: {
                var dataRangeX = myplotter.dataMaxX - myplotter.dataMinX
                if (dataRangeX <= 0.0) return 0.0
                return Math.max(0.0, Math.min(1.0 - size, (myplotter.panX - myplotter.dataMinX) / dataRangeX))
            }

            // Khi người dùng kéo thanh cuộn: cập nhật panX
            onPositionChanged: {
                if (activeFocus || pressed) {
                    var dataRangeX = myplotter.dataMaxX - myplotter.dataMinX
                    var newPanX = myplotter.dataMinX + position * dataRangeX
                    if (Math.abs(myplotter.panX - newPanX) > 1e-5) {
                        myplotter.panX = newPanX
                    }
                }
            }
        }
    }

    // Nhãn trục X dưới lề đồ thị
    Repeater {
        model: myplotter.chartType !== 2 ? myplotter.xTicks : null
        delegate: Text {
            text: modelData.value
            color: "#888888"
            font.pixelSize: 10
            font.family: "Monospace"
            x: myplotter.x + myplotter.dataToX(modelData.val) - width / 2
            y: myplotter.y + myplotter.height + 4
            visible: x >= myplotter.x - 2 && x + width <= myplotter.x + myplotter.width + 2
        }
    }

    // Nhãn trục Y bên lề trái đồ thị
    Repeater {
        model: myplotter.chartType !== 2 ? myplotter.yTicks : null
        delegate: Text {
            text: modelData.value
            color: "#888888"
            font.pixelSize: 10
            font.family: "Monospace"
            x: myplotter.x - width - 8
            y: myplotter.y + myplotter.dataToY(modelData.val) - height / 2
            visible: y >= myplotter.y - 2 && y + height <= myplotter.y + myplotter.height + 2
        }
    }

    Connections {
        target: myplotter
        function onDataModeChanged() {
            if (modeSelector.currentIndex !== myplotter.dataMode)
                modeSelector.currentIndex = myplotter.dataMode
        }
    }

    ColorDialog {
        id: colorDialog
        onAccepted: myplotter.chartColor = colorDialog.selectedColor
    }

    FileDialog {
        id: dataFileDialog
        title: qsTr("Chọn file dữ liệu")
        fileMode: FileDialog.OpenFile
        nameFilters: ["Text files (*.txt)", "All files (*)"]
        onAccepted: {
            if (!selectedFile)
                return
            var success = myplotter.loadDataFromFile(selectedFile)
            if (success) {
                console.log("QML: Da nap file va tinh toan Min/Max thanh cong!")
            } else {
                console.log("QML: Doc file that bai! Kiem tra lai duong dan.")
            }
        }
    }

    // Flow thay Row: tự xuống dòng, không bị tràn khỏi màn hình
    Flow {
        id: toolbar
        z: 1
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 12
        spacing: 8

        Button {
            text: "Nạp Dữ Liệu"
            enabled: myplotter.dataMode === 0
            background: Rectangle {
                implicitWidth: 130
                implicitHeight: 36
                color: parent.down ? "#d0d0d0" : (parent.hovered ? "#e0e0e0" : "#ffffff")
                border.color: "#999999"
                radius: 4
            }
            onClicked: dataFileDialog.open()
        }

        Button {
            text: "Xóa Dữ Liệu"
            background: Rectangle {
                implicitWidth: 120
                implicitHeight: 36
                color: parent.down ? "#d0d0d0" : (parent.hovered ? "#e0e0e0" : "#ffffff")
                border.color: "#999999"
                radius: 4
            }
            onClicked: myplotter.clearChart()
        }

        Button {
            text: "Chọn màu"
            background: Rectangle {
                implicitWidth: 110
                implicitHeight: 36
                color: parent.down ? "#d0d0d0" : (parent.hovered ? "#e0e0e0" : "#ffffff")
                border.color: "#999999"
                radius: 4
            }
            onClicked: colorDialog.open()
        }

        ComboBox {
            id: modeSelector
            width: 130
            model: ["Offline", "Online"]
        }

        Button {
            id: pauseResumeButton
            visible: myplotter.dataMode === 1
            text: isPaused ? "Tiếp tục" : "Tạm dừng"
            property bool isPaused: false

            Connections {
                target: myplotter
                function onDataModeChanged() {
                    pauseResumeButton.isPaused = false
                }
            }

            background: Rectangle {
                implicitWidth: 110
                implicitHeight: 36
                color: parent.down ? "#d0d0d0" : (parent.hovered ? "#e0e0e0" : "#ffffff")
                border.color: "#999999"
                radius: 4
            }

            onClicked: {
                if (isPaused) {
                    myplotter.resumeStream()
                    myplotter.isAutoPanEnabled = true
                    isPaused = false
                } else {
                    myplotter.pauseStream()
                    myplotter.isAutoPanEnabled = false
                    isPaused = true
                }
            }
        }

        ComboBox {
            id: typeSelector
            width: 140
            model: ["Line Chart", "Bar Chart", "Pie Chart"]
        }

        ComboBox {
            id: lineStyleSelector
            visible: typeSelector.currentIndex === 0
            width: 160
            model: ["Nét liền (Solid)", "Nét đứt (Dashed)", "Nét chấm (Dotted)"]
        }

        Button {
            visible: myplotter.dataMode === 0
            text: "Reset Zoom"
            background: Rectangle {
                implicitWidth: 110
                implicitHeight: 36
                color: parent.down ? "#d0d0d0" : (parent.hovered ? "#e0e0e0" : "#ffffff")
                border.color: "#999999"
                radius: 4
            }
            onClicked: myplotter.resetZoom()
        }

        Button {
            visible: myplotter.chartType !== 2
            text: myplotter.isCropMode ? "[X]" : "Chế độ Crop"
            background: Rectangle {
                implicitWidth: 130
                implicitHeight: 36
                color: parent.down ? "#d0d0d0" : (myplotter.isCropMode ? "#ffcccc" : (parent.hovered ? "#e0e0e0" : "#ffffff"))
                border.color: "#999999"
                radius: 4
            }
            onClicked: myplotter.isCropMode = !myplotter.isCropMode
        }

        Button {
            text: "Stress Test (1M)"
            background: Rectangle {
                implicitWidth: 150
                implicitHeight: 36
                color: parent.down ? "#d0d0d0" : (parent.hovered ? "#e0e0e0" : "#ffffff")
                border.color: "#999999"
                radius: 4
            }
            onClicked: myplotter.createStressTestData()
        }
    }

    Text {
        z: 1
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 12
        color: "#cccccc"
        font.pixelSize: 12
        text: "Offline: nạp file | Online: stream real-time | Pan/Zoom: chuột | Shift/Ctrl: trục zoom"
    }
}
