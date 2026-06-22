#include "datamanager.h"

DataManager::DataManager(QObject *parent) : QObject{parent} {
    // cài đặt thông số ban đầu khi chưa có dữ liệu nào
    m_minX=std::numeric_limits<float>::max(); // min khới tạo bằng số lớn nhất có thể
    m_maxX=std::numeric_limits<float>::lowest();// max khởi đầu bằng số nhỏ nhất
        m_minY=std::numeric_limits<float>::max();
    m_maxY=std::numeric_limits<float>::lowest();
}

DataManager *DataManager::instance()
{
    static DataManager _instance;
    return &_instance;
}

void DataManager::addData(float x, float y)
{
    std::lock_guard<std::mutex>lock(mutex);
    m_data.push_back({x,y});

    m_minX=std::min(m_minX,x);
    m_maxX=std::max(m_maxX,x);
    m_minY=std::min(m_minY,y);
    m_maxY=std::max(m_maxY,y);
    // phát tín hiệu cho những biểu đồ đang quan sát
    emit dataChanged();
}

std::vector<DataPoint> DataManager::getData()
{
    std::lock_guard<std::mutex> lock(mutex);
    return m_data;
}

void DataManager::clear()
{
   std::lock_guard<std::mutex> lock(mutex);
   m_data.clear();
   // khi xóa phải reset vè trạng thái ban đầu
   m_minX=std::numeric_limits<float>::max(); // min khới tạo bằng số lớn nhất có thể
   m_maxX=std::numeric_limits<float>::lowest();// max khởi đầu bằng số nhỏ nhất
   m_minY=std::numeric_limits<float>::max();
   m_maxY=std::numeric_limits<float>::lowest();
    emit dataChanged();
}


