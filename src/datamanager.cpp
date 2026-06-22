#include "datamanager.h"

DataManager::DataManager(QObject *parent) : QObject{parent} {}

DataManager *DataManager::instance()
{
    static DataManager _instance;
    return &_instance;
}

void DataManager::addData(float x, float y)
{
    std::lock_guard<std::mutex>lock(mutex);
    m_data.push_back({x,y});
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
    emit dataChanged();
}


