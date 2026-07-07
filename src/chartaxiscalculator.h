#ifndef CHARTAXISCALCULATOR_H
#define CHARTAXISCALCULATOR_H

#include <QVariantList>

// Lớp tiện ích chịu trách nhiệm tính toán bước nhảy lưới (Grid Step) và tọa độ các mốc (Ticks)
class ChartAxisCalculator
{
public:
    static float calculateGridStep(float range);
    static QVariantList calculateTicks(float minVal, float maxVal);
};

#endif // CHARTAXISCALCULATOR_H
