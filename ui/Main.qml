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
        chartColor: "#f1c40f"
        Behavior on chartColor {
            ColorAnimation { duration: 150 }
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

        Button {
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
