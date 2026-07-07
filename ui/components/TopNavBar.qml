import QtQuick
import QtQuick.Controls 2.15

Rectangle {
    id: navbar
    height: 64
    color: "#181e29"
    border.color: "#2a3441"
    border.width: 1

    property var myplotter
    signal openFileDialog()
    signal openColorDialog()

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
            color: myplotter && myplotter.dataMode === 1 ? "#1e3a2f" : "#1e293b"
            border.color: myplotter && myplotter.dataMode === 1 ? "#10b981" : "#475569"
            border.width: 1
            anchors.verticalCenter: parent.verticalCenter

            Text {
                id: statusText
                anchors.centerIn: parent
                text: myplotter && myplotter.dataMode === 1 ? "⚡ Online" : "🟢 Offline"
                color: myplotter && myplotter.dataMode === 1 ? "#34d399" : "#94a3b8"
                font.pixelSize: 11
                font.bold: true
            }
        }
    }

    // --- CONTROL TOOLBAR
    Row {
        anchors.right: parent.right
        anchors.rightMargin: 16
        anchors.verticalCenter: parent.verticalCenter
        spacing: 10

        // Nhóm 1: Nguồn dữ liệu (Data Source)
        Button {
            text: "📂 Nạp File"
            enabled: myplotter && myplotter.dataMode === 0
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
            onClicked: navbar.openFileDialog()
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
            onClicked: if (myplotter) myplotter.clearChart()
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
            onActivated: if (myplotter) myplotter.dataMode = currentIndex
        }

        Button {
            id: pauseResumeButton
            visible: myplotter && myplotter.dataMode === 1
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
                if (!myplotter) return
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
            onActivated: if (myplotter) myplotter.chartType = currentIndex
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
            onActivated: if (myplotter) myplotter.lineStyle = currentIndex
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

            Rectangle {
                width: 16; height: 16; radius: 8
                color: myplotter ? myplotter.chartColor : "#ffffff"
                border.color: "#ffffff"; border.width: 1.5
                anchors.right: parent.right; anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter
            }
            onClicked: navbar.openColorDialog()
        }

        // Divider
        Rectangle { width: 1; height: 26; color: "#2a3441"; anchors.verticalCenter: parent.verticalCenter }

        // Nhóm 3: Tương tác (Interactive Tools)
        Button {
            visible: myplotter && myplotter.chartType !== 2
            text: myplotter && myplotter.isCropMode ? "✂️ [Đang chọn]" : "✂️ Crop"
            background: Rectangle {
                implicitWidth: 115; implicitHeight: 34
                color: myplotter && myplotter.isCropMode ? (parent.down ? "#dc2626" : "#ef4444") : (parent.down ? "#1e293b" : (parent.hovered ? "#334155" : "#1e293b"))
                border.color: myplotter && myplotter.isCropMode ? "#f87171" : (parent.hovered ? "#475569" : "#334155")
                radius: 6
            }
            contentItem: Text {
                text: parent.text; font.pixelSize: 12; font.bold: true
                color: "#ffffff"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }
            onClicked: if (myplotter) myplotter.isCropMode = !myplotter.isCropMode
        }

        Button {
            visible: myplotter && myplotter.dataMode === 0
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
            onClicked: if (myplotter) myplotter.resetZoom()
        }
    }

    Connections {
        target: myplotter
        function onDataModeChanged() {
            if (myplotter && modeSelector.currentIndex !== myplotter.dataMode)
                modeSelector.currentIndex = myplotter.dataMode
        }
        function onChartTypeChanged() {
            if (myplotter && typeSelector.currentIndex !== myplotter.chartType)
                typeSelector.currentIndex = myplotter.chartType
        }
        function onLineStyleChanged() {
            if (myplotter && lineStyleSelector.currentIndex !== myplotter.lineStyle)
                lineStyleSelector.currentIndex = myplotter.lineStyle
        }
    }
}
