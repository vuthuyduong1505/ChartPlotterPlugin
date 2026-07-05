import QtQuick
import MyChartLibrary 1.0
import QtQuick.Controls 2.15
import QtQuick.Dialogs

Window {
    width: 1024
    height: 680
    visible: true
    title: qsTr("⚡ Qt OpenGL Chart Plotter Studio - High Performance Engine")
    color: "#12161f" // Modern Deep Navy / Slate Dark Mode

    // --- TOP NAVBAR & CONTROL HEADER ---
    Rectangle {
        id: navbar
        z: 10
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 64
        color: "#181e29"
        border.color: "#2a3441"
        border.width: 1

        Row {
            anchors.left: parent.left
            anchors.leftMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            spacing: 12

            Text {
                text: "⚡ Qt OpenGL Chart Plotter"
                color: "#ffffff"
                font.pixelSize: 16
                font.bold: true
            }

            Rectangle {
                width: statusText.width + 16
                height: 24
                radius: 12
                color: myplotter.dataMode === 1 ? "#1e3a2f" : "#1e293b"
                border.color: myplotter.dataMode === 1 ? "#10b981" : "#475569"
                border.width: 1
                anchors.verticalCenter: parent.verticalCenter

                Text {
                    id: statusText
                    anchors.centerIn: parent
                    text: myplotter.dataMode === 1 ? "⚡ Online" : "🟢 Offline"
                    color: myplotter.dataMode === 1 ? "#34d399" : "#94a3b8"
                    font.pixelSize: 11
                    font.bold: true
                }
            }
        }

        // --- CONTROL TOOLBAR (RIGHT ALIGNED / WRAP CAPABLE) ---
        Row {
            anchors.right: parent.right
            anchors.rightMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            spacing: 10

            // Nhóm 1: Nguồn dữ liệu (Data Source)
            Button {
                text: "📂 Nạp File"
                enabled: myplotter.dataMode === 0
                background: Rectangle {
                    implicitWidth: 95
                    implicitHeight: 34
                    color: parent.down ? "#1d4ed8" : (parent.hovered ? "#2563eb" : "#3b82f6")
                    border.color: parent.hovered ? "#60a5fa" : "#2563eb"
                    radius: 6
                }
                contentItem: Text {
                    text: parent.text; font.pixelSize: 12; font.bold: true
                    color: parent.enabled ? "#ffffff" : "#64748b"
                    horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                }
                onClicked: dataFileDialog.open()
            }

            Button {
                text: "🗑️ Xóa"
                background: Rectangle {
                    implicitWidth: 70
                    implicitHeight: 34
                    color: parent.down ? "#7f1d1d" : (parent.hovered ? "#991b1b" : "#1e293b")
                    border.color: parent.hovered ? "#ef4444" : "#334155"
                    radius: 6
                }
                contentItem: Text {
                    text: parent.text; font.pixelSize: 12; font.bold: true
                    color: "#ffffff"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                }
                onClicked: myplotter.clearChart()
            }

            ComboBox {
                id: modeSelector
                width: 100
                height: 34
                model: ["Offline", "Online"]
                background: Rectangle {
                    color: parent.down ? "#1e293b" : (parent.hovered ? "#334155" : "#1e293b")
                    border.color: parent.hovered ? "#475569" : "#334155"; radius: 6
                }
                contentItem: Text {
                    leftPadding: 10; text: parent.displayText; font.pixelSize: 12; font.bold: true
                    color: "#ffffff"; verticalAlignment: Text.AlignVCenter; elide: Text.ElideRight
                }
            }

            Button {
                id: pauseResumeButton
                visible: myplotter.dataMode === 1
                text: isPaused ? "▶ Tiếp tục" : "⏸ Tạm dừng"
                property bool isPaused: false
                background: Rectangle {
                    implicitWidth: 95; implicitHeight: 34
                    color: parent.down ? "#b45309" : (parent.hovered ? "#d97706" : "#f59e0b")
                    border.color: "#fbbf24"; radius: 6
                }
                contentItem: Text {
                    text: parent.text; font.pixelSize: 12; font.bold: true
                    color: "#ffffff"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
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
                Connections {
                    target: myplotter
                    function onDataModeChanged() { pauseResumeButton.isPaused = false }
                }
            }

            // Divider
            Rectangle { width: 1; height: 26; color: "#2a3441"; anchors.verticalCenter: parent.verticalCenter }

            // Nhóm 2: Loại biểu đồ & Thẩm mỹ (Chart Styling)
            ComboBox {
                id: typeSelector
                width: 120; height: 34
                model: ["Line Chart", "Bar Chart", "Pie Chart"]
                background: Rectangle {
                    color: parent.down ? "#1e293b" : (parent.hovered ? "#334155" : "#1e293b")
                    border.color: parent.hovered ? "#475569" : "#334155"; radius: 6
                }
                contentItem: Text {
                    leftPadding: 10; text: parent.displayText; font.pixelSize: 12; font.bold: true
                    color: "#ffffff"; verticalAlignment: Text.AlignVCenter; elide: Text.ElideRight
                }
            }

            ComboBox {
                id: lineStyleSelector
                visible: typeSelector.currentIndex === 0
                width: 140; height: 34
                model: ["Nét liền (Solid)", "Nét đứt (Dashed)", "Nét chấm (Dotted)"]
                background: Rectangle {
                    color: parent.down ? "#1e293b" : (parent.hovered ? "#334155" : "#1e293b")
                    border.color: parent.hovered ? "#475569" : "#334155"; radius: 6
                }
                contentItem: Text {
                    leftPadding: 10; text: parent.displayText; font.pixelSize: 12; font.bold: true
                    color: "#ffffff"; verticalAlignment: Text.AlignVCenter; elide: Text.ElideRight
                }
            }

            Button {
                text: "🎨 Màu  "
                background: Rectangle {
                    implicitWidth: 85; implicitHeight: 34
                    color: parent.down ? "#1e293b" : (parent.hovered ? "#334155" : "#1e293b")
                    border.color: parent.hovered ? "#475569" : "#334155"; radius: 6
                }
                contentItem: Text {
                    leftPadding: 8; text: parent.text; font.pixelSize: 12; font.bold: true
                    color: "#ffffff"; verticalAlignment: Text.AlignVCenter
                }
                // Color Preview Dot
                Rectangle {
                    width: 16; height: 16; radius: 8
                    color: myplotter.chartColor
                    border.color: "#ffffff"; border.width: 1.5
                    anchors.right: parent.right; anchors.rightMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                }
                onClicked: colorDialog.open()
            }

            // Divider
            Rectangle { width: 1; height: 26; color: "#2a3441"; anchors.verticalCenter: parent.verticalCenter }

            // Nhóm 3: Tương tác (Interactive Tools)
            Button {
                visible: myplotter.chartType !== 2
                text: myplotter.isCropMode ? "✂️ [Đang chọn]" : "✂️ Crop"
                background: Rectangle {
                    implicitWidth: 115; implicitHeight: 34
                    color: myplotter.isCropMode ? (parent.down ? "#dc2626" : "#ef4444") : (parent.down ? "#1e293b" : (parent.hovered ? "#334155" : "#1e293b"))
                    border.color: myplotter.isCropMode ? "#f87171" : (parent.hovered ? "#475569" : "#334155")
                    radius: 6
                }
                contentItem: Text {
                    text: parent.text; font.pixelSize: 12; font.bold: true
                    color: "#ffffff"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                }
                onClicked: myplotter.isCropMode = !myplotter.isCropMode
            }

            Button {
                visible: myplotter.dataMode === 0
                text: "🔄 Reset"
                background: Rectangle {
                    implicitWidth: 80; implicitHeight: 34
                    color: parent.down ? "#1e293b" : (parent.hovered ? "#334155" : "#1e293b")
                    border.color: parent.hovered ? "#475569" : "#334155"; radius: 6
                }
                contentItem: Text {
                    text: parent.text; font.pixelSize: 12; font.bold: true
                    color: "#ffffff"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                }
                onClicked: myplotter.resetZoom()
            }
        }
    }

    // --- DIALOGS & CONNECTIONS ---
    Connections {
        target: myplotter
        function onDataModeChanged() {
            if (modeSelector.currentIndex !== myplotter.dataMode)
                modeSelector.currentIndex = myplotter.dataMode
        }
        function onChartTypeChanged() {
            if (typeSelector.currentIndex !== myplotter.chartType)
                typeSelector.currentIndex = myplotter.chartType
        }
        function onLineStyleChanged() {
            if (lineStyleSelector.currentIndex !== myplotter.lineStyle)
                lineStyleSelector.currentIndex = myplotter.lineStyle
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
            if (!selectedFile) return
            var success = myplotter.loadDataFromFile(selectedFile)
            if (success) {
                console.log("QML: Da nap file va tinh toan Min/Max thanh cong!")
            } else {
                console.log("QML: Doc file that bai! Kiem tra lai duong dan.")
            }
        }
    }

    // --- CHART WORKSPACE CARD ---
    Rectangle {
        id: chartCard
        z: 1
        anchors.top: navbar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: footerBar.top
        anchors.margins: 16
        color: "#181e29"
        border.color: "#2a3441"
        border.width: 1.5
        radius: 12
        clip: true

        MyChart {
            id: myplotter
            anchors.fill: parent
            anchors.topMargin: 15
            anchors.leftMargin: 45
            anchors.rightMargin: 25
            anchors.bottomMargin: 48
            chartType: typeSelector.currentIndex
            dataMode: modeSelector.currentIndex
            lineStyle: lineStyleSelector.currentIndex
            chartColor: "#f1c40f"
            Behavior on chartColor {
                ColorAnimation { duration: 150 }
            }

            property bool isCropMode: false

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

            readonly property var nearestPointMap: myplotter.getNearestDataPoint(
                hoverHandler.point.position.x,
                hoverHandler.point.position.y,
                myplotter.width,
                myplotter.height
            )

            HoverHandler {
                id: hoverHandler
            }

            // Chấm tròn tiêu điểm tại điểm bắt dính
            Rectangle {
                id: snapDot
                width: 12; height: 12; radius: 6
                color: "#f1c40f"; border.color: "#ffffff"; border.width: 1.5
                x: myplotter.nearestPointMap.valid ? myplotter.dataToX(myplotter.nearestPointMap.dataX) - width / 2 : 0
                y: myplotter.nearestPointMap.valid ? myplotter.dataToY(myplotter.nearestPointMap.dataY) - height / 2 : 0
                visible: hoverHandler.hovered && myplotter.nearestPointMap.valid && !myplotter.nearestPointMap.isPie
                z: 8
            }

            // Crosshair dọc
            Rectangle {
                id: crosshairVertical
                width: 1; height: parent.height; color: "#40ffffff"
                x: myplotter.nearestPointMap.valid ? myplotter.nearestPointMap.screenX : 0
                visible: hoverHandler.hovered && myplotter.nearestPointMap.valid && !myplotter.nearestPointMap.isPie
                z: 5
            }

            // Crosshair ngang
            Rectangle {
                id: crosshairHorizontal
                width: parent.width; height: 1; color: "#40ffffff"
                y: myplotter.nearestPointMap.valid ? myplotter.nearestPointMap.screenY : 0
                visible: hoverHandler.hovered && myplotter.nearestPointMap.valid && !myplotter.nearestPointMap.isPie
                z: 5
            }

            // Tooltip hiển thị thông tin
            Rectangle {
                id: tooltip
                color: "#f2181e29"
                border.color: "#475569"; border.width: 1; radius: 8
                visible: hoverHandler.hovered && myplotter.nearestPointMap.valid
                z: 10
                width: tooltipLayout.width + 20; height: tooltipLayout.height + 16

                x: {
                    var mx = myplotter.nearestPointMap.valid ? myplotter.nearestPointMap.screenX : 0;
                    var targetX = mx + 14;
                    if (targetX + width > parent.width) targetX = mx - width - 14;
                    return Math.max(4, Math.min(parent.width - width - 4, targetX));
                }
                y: {
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
                        text: myplotter.nearestPointMap.valid 
                              ? (myplotter.nearestPointMap.isPie 
                                 ? "Tỷ lệ: " + (myplotter.nearestPointMap.percent < 0.01 && myplotter.nearestPointMap.percent > 0 ? "<0.01" : myplotter.nearestPointMap.percent.toFixed(1)) + "% (" + myplotter.nearestPointMap.dataY + " điểm)"
                                 : "X: " + myplotter.nearestPointMap.dataX.toFixed(3)) 
                              : ""
                    }

                    Text {
                        color: "#f1c40f"; font.pixelSize: 12; font.bold: true; font.family: "Monospace"
                        visible: myplotter.nearestPointMap.valid && !myplotter.nearestPointMap.isPie
                        text: myplotter.nearestPointMap.valid && !myplotter.nearestPointMap.isPie 
                              ? "Y: " + myplotter.nearestPointMap.dataY.toFixed(3) : ""
                    }

                    Text {
                        color: "#34d399"; font.pixelSize: 11; font.bold: true; font.family: "Monospace"
                        visible: myplotter.nearestPointMap.valid && myplotter.nearestPointMap.isPie
                        text: {
                            if (!myplotter.nearestPointMap.valid || !myplotter.nearestPointMap.isPie) return "";
                            if (myplotter.nearestPointMap.sliceName === "Ngoại lai (Outliers)") return "Khoảng giá trị: Khác";
                            var minVal = myplotter.nearestPointMap.currentBinMin;
                            var maxVal = myplotter.nearestPointMap.currentBinMax;
                            return "Khoảng giá trị: " + (minVal !== undefined ? minVal.toFixed(3) : "0.000") + " -> " + (maxVal !== undefined ? maxVal.toFixed(3) : "0.000");
                        }
                    }
                }
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
                enabled: myplotter.isCropMode
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
                    var curX = Math.max(0, Math.min(myplotter.width, mouse.x))
                    var curY = Math.max(0, Math.min(myplotter.height, mouse.y))
                    cropRect.x = Math.min(startX, curX); cropRect.y = Math.min(startY, curY)
                    cropRect.width = Math.abs(curX - startX); cropRect.height = Math.abs(curY - startY)
                }

                onReleased: (mouse) => {
                    cropRect.visible = false
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
                visible: myplotter.chartType !== 2 && (myplotter.dataMaxX > myplotter.dataMinX)
                size: (myplotter.zoomX > 0.0) ? (1.0 / myplotter.zoomX) : 1.0
                position: {
                    var dataRangeX = myplotter.dataMaxX - myplotter.dataMinX
                    if (dataRangeX <= 0.0) return 0.0
                    return Math.max(0.0, Math.min(1.0 - size, (myplotter.panX - myplotter.dataMinX) / dataRangeX))
                }
                onPositionChanged: {
                    if (activeFocus || pressed) {
                        var dataRangeX = myplotter.dataMaxX - myplotter.dataMinX
                        var newPanX = myplotter.dataMinX + position * dataRangeX
                        if (Math.abs(myplotter.panX - newPanX) > 1e-5) myplotter.panX = newPanX
                    }
                }
            }
        }

        // Nhãn trục X dưới lề đồ thị
        Repeater {
            model: myplotter.chartType !== 2 ? myplotter.xTicks : null
            delegate: Text {
                text: modelData.value; color: "#94a3b8"; font.pixelSize: 10; font.family: "Monospace"
                x: myplotter.x + myplotter.dataToX(modelData.val) - width / 2
                y: myplotter.y + myplotter.height + 6
                visible: x >= myplotter.x - 2 && x + width <= myplotter.x + myplotter.width + 2
            }
        }

        // Nhãn trục Y bên lề trái đồ thị
        Repeater {
            model: myplotter.chartType !== 2 ? myplotter.yTicks : null
            delegate: Text {
                text: modelData.value; color: "#94a3b8"; font.pixelSize: 10; font.family: "Monospace"
                x: myplotter.x - width - 10
                y: myplotter.y + myplotter.dataToY(modelData.val) - height / 2
                visible: y >= myplotter.y - 2 && y + height <= myplotter.y + myplotter.height + 2
            }
        }
    }

    // --- SMART TELEMETRY FOOTER ---
    Rectangle {
        id: footerBar
        z: 10
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 36
        color: "#151b26"
        border.color: "#232d3d"
        border.width: 1

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

        // --- LIVE CURSOR TELEMETRY ---
        Row {
            anchors.right: parent.right
            anchors.rightMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            spacing: 8

            Rectangle {
                width: 8; height: 8; radius: 4
                color: myplotter.nearestPointMap.valid ? "#10b981" : "#64748b"
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: {
                    if (!myplotter.nearestPointMap.valid) return "📍 Tọa độ: ---"
                    if (myplotter.nearestPointMap.isPie) {
                        return "📍 Lát cắt: " + myplotter.nearestPointMap.sliceName + " (" + myplotter.nearestPointMap.percent.toFixed(1) + "%)"
                    }
                    return "📍 Tọa độ: X = " + myplotter.nearestPointMap.dataX.toFixed(3) + "  |  Y = " + myplotter.nearestPointMap.dataY.toFixed(3)
                }
                color: myplotter.nearestPointMap.valid ? "#38bdf8" : "#64748b"
                font.pixelSize: 12
                font.bold: true
                font.family: "Monospace"
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
}
