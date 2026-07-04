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
};

#endif // DATAPROCESSOR_H
