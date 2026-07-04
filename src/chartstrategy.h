#ifndef CHARTSTRATEGY_H
#define CHARTSTRATEGY_H
#include <QOpenGLFunctions>
#include <QColor>
#include "src/datamanager.h"
#include <vector>

// đây là bản thiết kế chung cho các loại biểu đồ
class ChartStrategy{
public:
    virtual ~ChartStrategy(){}
    virtual void init() =0; // hàm khởi tạo, nạp shader và tạo vbo,vao
    virtual void draw(QOpenGLFunctions *f, float time, const QColor &color,
                      const std::vector<DataPoint> &rawData,
                      float minX, float maxX, float minY, float maxY,
                      bool dataDirty) =0; // thực hiện lệnh vẽ biểu đồ

};

#endif // CHARTSTRATEGY_H
