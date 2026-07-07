import QtQuick
import MyChartLibrary 1.0
import QtQuick.Controls 2.15
import QtQuick.Dialogs
import "components"

Window {
    width: 1024
    height: 680
    visible: true
    title: qsTr("⚡ Qt OpenGL Chart Plotter")
    color: "#12161f" // Modern Deep Navy / Slate Dark Mode

    // --- TOP NAVBAR & CONTROL HEADER ---
    TopNavBar {
        id: navbar
        z: 10
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        myplotter: myplotter
        onOpenFileDialog: dataFileDialog.open()
        onOpenColorDialog: colorDialog.open()
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

            CrosshairOverlay {
                myplotter: myplotter
                hoverHandler: hoverHandler
            }

            ChartTooltip {
                myplotter: myplotter
                hoverHandler: hoverHandler
            }
        }

        AxisLabels {
            myplotter: myplotter
        }
    }

    FooterStatusBar {
        id: footerBar
        z: 10
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        myplotter: myplotter
    }
}
