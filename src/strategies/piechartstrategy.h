#ifndef PIECHARTSTRATEGY_H
#define PIECHARTSTRATEGY_H

#include "src/chartstrategy.h"
#include "src/datamanager.h"
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <vector>

class pieChartStrategy : public ChartStrategy
{
public:
    pieChartStrategy();
    ~pieChartStrategy() override;
    void init() override;
    void draw(QOpenGLFunctions *f, float time, const QColor &color,
              const std::vector<DataPoint> &rawData,
              float minX, float maxX, float minY, float maxY,
              bool dataDirty) override;

private:
    // TỐI ƯU: Sử dụng 3 VBO cho 3 lát bánh riêng biệt, tránh allocation mỗi frame
    QOpenGLBuffer vboPie[3];
    int m_vertexCount[3] = {0, 0, 0};
    QOpenGLShaderProgram *program;
    QOpenGLVertexArrayObject vao;
};

#endif // PIECHARTSTRATEGY_H
