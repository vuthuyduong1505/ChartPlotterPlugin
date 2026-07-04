#ifndef BARCHARTSTRATEGY_H
#define BARCHARTSTRATEGY_H

#include "src/chartstrategy.h"
#include "src/datamanager.h"
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <vector>

class BarChartStrategy : public ChartStrategy
{
public:
    BarChartStrategy();
    ~BarChartStrategy() override;
    void init() override;
    void draw(QOpenGLFunctions *f, float time, const QColor &color,
              const std::vector<DataPoint> &rawData,
              float minX, float maxX, float minY, float maxY,
              bool dataDirty, int lineStyle = 0) override;
private:
    QOpenGLShaderProgram *program;
    QOpenGLBuffer vboBar;
    QOpenGLVertexArrayObject vao;
    std::vector<float> barData;

};

#endif // BARCHARTSTRATEGY_H
