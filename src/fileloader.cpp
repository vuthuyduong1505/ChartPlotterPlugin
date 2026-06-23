#include "fileloader.h"
#include "datamanager.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>

bool FileLoader::loadDataset(const QString &filePath)
{
  //Mở của kết nối với file
  //QFile là class của Qt đại diện cho một file vật lí trên ổ đĩa
  QFile file(filePath);

  //lệnh open() mở file dưới chế độ chỉ đọc ReadOnly và định dạng văn bản Text
//Nếu mở thất bại hàm trả về false
  if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
      qWarning()<<"khong the mo filetai duong dan"<<filePath;
      return false;
  }
  // tạo 1 vector để gom toàn bộ dữ liệu tĩnh trong file
  std::vector<DataPoint> buffer;
  //sau khi nập file xóa dữ kiệu cũ để chuẩn bị nạp dữ liệu mới

  //Tạo đường ống đọc chữ
  //QTextStream là bộ công cụ của Qt để bóc tách chữ từ file
  QTextStream in(&file);

  //Vòng lặp quét từ đầu đến cuối file
  //Hàm atEnd() trả về true khi máy đọc đến dòng cuối cảu file
  while(!in.atEnd()){
      //dọc nguyên dòng hiện tại và chuyển con trỏ xuống dòng tiếp theo
      QString line = in.readLine();

      //Sửa lỗi nếu người dùng lỡ tay bấm phím cách hoặc xuống dòng thừa ở cuối file
      if(line.trimmed().isEmpty())
      {
          continue;
      }
      //Cắt chuỗi
      //hàm spli sẽ tìm các dấu cách tròn dòng để cắt chuỗi và trả về một danh sách các chuỗi
      QStringList parts=line.split(' ');

      if(parts.size()>=2)
      {
          for(int i=0;i<parts.size()-1;i+=2)
          {
          float x=parts[i].toFloat();
          float y=parts[i+1].toFloat();

          buffer.push_back({x,y});
          }
      }
  }
  //Đóng file
  file.close();
  DataManager::instance()->setData(buffer);
  qDebug()<<"Nap file thanh cong";
  return true;
}
