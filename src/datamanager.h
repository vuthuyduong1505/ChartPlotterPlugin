#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <vector>
#include<mutex>
struct DataPoint{
    float x;
    float y;
};

class DataManager : public QObject
{
    Q_OBJECT
public:
    // hàm lấy instance duy nhất (Singleton Pattern)
    static DataManager* instance();

    // Hàm nạp thêm dữ liệu vào kho
    void addData(float x, float y);

    // hàm lấy toàn bộ dữ liệu ra để vẽ
    std::vector<DataPoint> getData();
    //hàm xóa sạch kho
    void clear();

signals:
    // báo hiệu dữ liệu đã thay đổi
    void dataChanged();

private:
    explicit DataManager(QObject *parent = nullptr);
    std::vector<DataPoint> m_data;
    std::mutex mutex;

};

#endif // DATAMANAGER_H
