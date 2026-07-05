#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include "src/datamanager.h"
#include <vector>

class DataProcessor
{
public:
    static void calculateBounds(const std::vector<DataPoint>& data, 
                                float& minX, float& maxX, 
                                float& minY, float& maxY);
    static std::vector<DataPoint> downsampleLTTB(const std::vector<DataPoint>& data, int threshold);
    static float calculateBarWidth(const std::vector<DataPoint>& data, float minX, float maxX);
};

#endif // DATAPROCESSOR_H
