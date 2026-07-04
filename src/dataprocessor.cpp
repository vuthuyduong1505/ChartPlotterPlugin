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
