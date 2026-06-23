import QtQuick
import MyChartLibrary 1.0
import QtQuick.Controls 2.15 // thu viện nút bấm
Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Test QML")

    MyChart {
          id: myplotter
           anchors.fill: parent // Cho biểu đồ tràn hết cửa sổ
           chartType: typeSelector.currentIndex // lấy giá trị loại biểu đồ đnag chọn cho vào biến chartType

       }
     Row{
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 20
            spacing: 10 // Khoảng cách giữa combo box và nút bấm
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
       //nút bấm nạp file
       Button {
                   text: "Nạp Dữ Liệu"

                   // Định dạng một chút cho đẹp (Tùy chọn)
                   background: Rectangle {
                       implicitWidth: 150
                       implicitHeight: 40
                       color: parent.down ? "#d0d0d0" : (parent.hovered ? "#e0e0e0" : "#ffffff")
                       border.color: "#999999"
                       radius: 4
                   }

       // HÀNH ĐỘNG KHI BẤM NÚT:
                   onClicked: {
                       // Ta gọi thẳng tên hàm Q_INVOKABLE đã mở cổng ở C++ xuống:

                       var success = myplotter.loadDataFromFile("D:/QT/Project/Test/data.txt");

                       if (success) {
                           console.log("QML: Da nap file va tinh toan Min/Max thanh cong!");
                       } else {
                           console.log("QML: Doc file that bai! Kiểm tra lại đường dẫn.");
                       }
                   }
               }
}



