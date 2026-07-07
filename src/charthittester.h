#ifndef CHARTHITTESTER_H
#define CHARTHITTESTER_H

#include <QVariantMap>
#include <QColor>
#include "src/viewportmanager.h"

// Lớp phụ trách xử lý toán học va chạm (Hit-Testing / Ray-casting) cho các loại biểu đồ
class ChartHitTester
{
public:
    static QVariantMap getNearestDataPoint(float mouseX, float mouseY, 
                                           float screenWidth, float screenHeight,
                                           int chartType, int pieBinMode, 
                                           const QColor &chartColor,
                                           const ViewportManager &viewport, 
                                           int &hoveredSlice, bool &sliceChanged);
};

#endif // CHARTHITTESTER_H
