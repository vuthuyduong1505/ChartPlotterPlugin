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
    // Hàm nạp cả mảng dữ liệu
    void setData(const std::vector<DataPoint> &newData);
    //hàm xóa sạch kho
    void clear();

    // Các hàm phụ trợ tối ưu hóa Pointing tránh copy mảng lớn
    bool isEmpty();
    int dataSize();
    DataPoint firstPoint();
    DataPoint lastPoint();
    DataPoint findNearestPoint(float targetX);
    std::pair<DataPoint, DataPoint> findAdjacentPoints(float targetX);
    void createStressTestData();


signals:
    // báo hiệu dữ liệu đã thay đổi
    void dataChanged();

private:
    explicit DataManager(QObject *parent = nullptr);
    std::vector<DataPoint> m_data;
    std::mutex mutex;

};

#endif // DATAMANAGER_H
