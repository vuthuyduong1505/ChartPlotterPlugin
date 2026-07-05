#ifndef CHARTSTRATEGYFACTORY_H
#define CHARTSTRATEGYFACTORY_H

#include "src/chartstrategy.h"

class ChartStrategyFactory
{
public:
    enum ChartType {
        LineChart = 0,
        BarChart = 1,
        PieChart = 2
    };

    // Hàm Factory chính khởi tạo Strategy dựa theo định dạng biểu đồ (type)
    static ChartStrategy* createStrategy(int type);
};

#endif // CHARTSTRATEGYFACTORY_H
