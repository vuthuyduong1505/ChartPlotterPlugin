import QtQuick
import MyChartLibrary 1.0
import QtQuick.Controls 2.15 // thu viện nút bấm
Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Test QML")

    MyChart {
        // id: myplotter
           anchors.fill: parent // Cho biểu đồ tràn hết cửa sổ
           chartType: typeSelector.currentIndex // lấy giá trị loại biểu đồ đnag chọn cho vào biến chartType

       }
      // menu chọn loại biểu đồ
       ComboBox{
           id:typeSelector
           width: 150
           model: ["Line Chart", "Bar Chart","Pie Chart"] // line:0, bar:1
           anchors.top: parent.top
           anchors.right: parent.right
           anchors.margins: 20
       }
       }

