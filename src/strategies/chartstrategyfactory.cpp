#include "chartstrategyfactory.h"
#include "linechartstrategy.h"
#include "barchartstrategy.h"
#include "piechartstrategy.h"

ChartStrategy* ChartStrategyFactory::createStrategy(int type)
{
    switch (type) {
    case LineChart:
        return new LineChartStrategy();
    case BarChart:
        return new BarChartStrategy();
    case PieChart:
        return new pieChartStrategy();
    default:
        return nullptr;
    }
}
