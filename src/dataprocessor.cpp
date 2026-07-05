#include "dataprocessor.h"
#include <algorithm>
#include <limits>
#include <cmath>

void DataProcessor::calculateBounds(const std::vector<DataPoint>& data, 
                                    float& minX, float& maxX, 
                                    float& minY, float& maxY)
{
    if (data.empty()) {
        minX = 0.0f;
        maxX = 1.0f;
        minY = 0.0f;
        maxY = 1.0f;
        return;
    }

    minX = std::numeric_limits<float>::max();
    maxX = std::numeric_limits<float>::lowest();
    minY = std::numeric_limits<float>::max();
    maxY = std::numeric_limits<float>::lowest();

    for (const auto& p : data) {
        minX = std::min(minX, p.x);
        maxX = std::max(maxX, p.x);
        minY = std::min(minY, p.y);
        maxY = std::max(maxY, p.y);
    }

    if (std::abs(maxX - minX) < 0.0001f) {
        maxX += 0.001f;
    }
    if (std::abs(maxY - minY) < 0.0001f) {
        maxY += 0.001f;
    }
}

std::vector<DataPoint> DataProcessor::downsampleLTTB(const std::vector<DataPoint>& data, int threshold)
{
    int dataSize = static_cast<int>(data.size());
    if (dataSize <= threshold || threshold <= 2) {
        return data;
    }

    std::vector<DataPoint> sampled;
    sampled.reserve(threshold);

    // 1. Điểm đầu luôn được chọn
    sampled.push_back(data[0]);

    // Kích thước trung bình của mỗi bucket (trừ điểm đầu và cuối)
    double bucketSize = static_cast<double>(dataSize - 2) / (threshold - 2);

    int aIdx = 0; // Chỉ số của điểm đã chọn ở bucket trước

    for (int i = 0; i < threshold - 2; ++i) {
        // Tính toán khoảng chỉ số của bucket hiện tại (i) và bucket tiếp theo (i+1)
        int bucketStart = static_cast<int>(std::floor((i) * bucketSize)) + 1;
        int bucketEnd = static_cast<int>(std::floor((i + 1) * bucketSize)) + 1;
        bucketEnd = std::min(bucketEnd, dataSize - 1);

        int nextBucketStart = static_cast<int>(std::floor((i + 1) * bucketSize)) + 1;
        int nextBucketEnd = static_cast<int>(std::floor((i + 2) * bucketSize)) + 1;
        nextBucketEnd = std::min(nextBucketEnd, dataSize - 1);

        // Tính điểm trung bình (centroid) của bucket tiếp theo
        double nextAvgX = 0.0;
        double nextAvgY = 0.0;
        int nextCount = nextBucketEnd - nextBucketStart;
        if (nextCount > 0) {
            for (int k = nextBucketStart; k < nextBucketEnd; ++k) {
                nextAvgX += data[k].x;
                nextAvgY += data[k].y;
            }
            nextAvgX /= nextCount;
            nextAvgY /= nextCount;
        } else {
            nextAvgX = data[dataSize - 1].x;
            nextAvgY = data[dataSize - 1].y;
        }

        // Tìm điểm c trong bucket hiện tại tối đa hóa diện tích tam giác (a, c, centroid)
        double maxArea = -1.0;
        int bestIdx = bucketStart;

        double ax = data[aIdx].x;
        double ay = data[aIdx].y;

        for (int k = bucketStart; k < bucketEnd; ++k) {
            double cx = data[k].x;
            double cy = data[k].y;
            double area = std::abs((ax - nextAvgX) * (cy - ay) - (ax - cx) * (nextAvgY - ay));
            if (area > maxArea) {
                maxArea = area;
                bestIdx = k;
            }
        }

        sampled.push_back(data[bestIdx]);
        aIdx = bestIdx; // Cập nhật điểm neo cho vòng lặp kế tiếp
    }

    // 2. Điểm cuối luôn được chọn
    sampled.push_back(data[dataSize - 1]);

    return sampled;
}

float DataProcessor::calculateBarWidth(const std::vector<DataPoint>& data, float minX, float maxX)
{
    if (data.size() < 2) {
        float range = maxX - minX;
        return (range <= 0.0f ? 1.0f : range) * 0.015f;
    }
    float minDeltaX = std::numeric_limits<float>::max();
    size_t limit = std::min(data.size(), static_cast<size_t>(2000));
    for (size_t i = 1; i < limit; ++i) {
        float dx = std::abs(data[i].x - data[i - 1].x);
        if (dx > 1e-6f && dx < minDeltaX) {
            minDeltaX = dx;
        }
    }
    if (minDeltaX == std::numeric_limits<float>::max() || minDeltaX <= 0.0f) {
        float range = maxX - minX;
        return (range <= 0.0f ? 1.0f : range) * 0.015f;
    }
    return 0.35f * minDeltaX;
}
