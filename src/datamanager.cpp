#include "datamanager.h"
#include <cmath>

DataManager::DataManager(QObject *parent) : QObject{parent} {
}

DataManager *DataManager::instance()
{
    static DataManager _instance;
    return &_instance;
}

void DataManager::addData(float x, float y)
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        m_data.push_back({x, y});
        if (m_data.size() > 2000) {
            // Sliding window: khi vượt quá 2000 điểm thì tự động xóa bỏ các điểm ở đầu mảng
            m_data.erase(m_data.begin(), m_data.begin() + (m_data.size() - 2000));
        }
    }
    emit dataChanged();
}

std::vector<DataPoint> DataManager::getData()
{
    std::lock_guard<std::mutex> lock(mutex);
    return m_data;
}

void DataManager::setData(const std::vector<DataPoint> &newData)
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        m_data=newData;
    }
    emit dataChanged();
}

void DataManager::clear()
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        m_data.clear();
    }
    emit dataChanged();
}

bool DataManager::isEmpty()
{
    std::lock_guard<std::mutex> lock(mutex);
    return m_data.empty();
}

int DataManager::dataSize()
{
    std::lock_guard<std::mutex> lock(mutex);
    return static_cast<int>(m_data.size());
}

DataPoint DataManager::firstPoint()
{
    std::lock_guard<std::mutex> lock(mutex);
    return m_data.empty() ? DataPoint{0.0f, 0.0f} : m_data.front();
}

DataPoint DataManager::lastPoint()
{
    std::lock_guard<std::mutex> lock(mutex);
    return m_data.empty() ? DataPoint{0.0f, 0.0f} : m_data.back();
}

DataPoint DataManager::findNearestPoint(float targetX)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (m_data.empty()) {
        return DataPoint{0.0f, 0.0f};
    }
    
    auto it = std::lower_bound(m_data.begin(), m_data.end(), targetX,
        [](const DataPoint &dp, float val) {
            return dp.x < val;
        });

    if (it == m_data.begin()) {
        return m_data.front();
    }
    if (it == m_data.end()) {
        return m_data.back();
    }

    const DataPoint &p2 = *it;
    const DataPoint &p1 = *(it - 1);
    if (std::abs(p2.x - targetX) < std::abs(p1.x - targetX)) {
        return p2;
    }
    return p1;
}

std::pair<DataPoint, DataPoint> DataManager::findAdjacentPoints(float targetX)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (m_data.empty()) {
        return { DataPoint{0.0f, 0.0f}, DataPoint{0.0f, 0.0f} };
    }
    if (m_data.size() == 1) {
        return { m_data[0], m_data[0] };
    }
    
    auto it = std::lower_bound(m_data.begin(), m_data.end(), targetX,
        [](const DataPoint &dp, float val) {
            return dp.x < val;
        });

    if (it == m_data.begin()) {
        return { m_data[0], m_data[1] };
    }
    if (it == m_data.end()) {
        return { m_data[m_data.size() - 2], m_data[m_data.size() - 1] };
    }
    return { *(it - 1), *it };
}

void DataManager::createStressTestData()
{
    std::vector<DataPoint> temp;
    temp.reserve(1000000);

    for (int i = 0; i < 1000000; ++i) {
        float x = static_cast<float>(i) * 0.01f;
        float y = std::sin(x);
        
        // Cứ mỗi 100.000 điểm, chèn một điểm Spike có giá trị vọt lên cao (10.0f)
        if (i > 0 && i % 100000 == 0) {
            y = 10.0f;
        }
        
        temp.push_back({x, y});
    }

    setData(temp);
}




