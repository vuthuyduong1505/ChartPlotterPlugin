#ifndef LINECHARTSTRATEGY_H
#define LINECHARTSTRATEGY_H

#include "src/chartstrategy.h"
#include "src/datamanager.h"
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <vector>

class LineChartStrategy : public ChartStrategy
{
public:
    LineChartStrategy();
    ~LineChartStrategy() override;

    void init() override;
    void draw(QOpenGLFunctions *f, float time, const QColor &color,
              const std::vector<DataPoint> &rawData,
              float minX, float maxX, float minY, float maxY,
              bool dataDirty) override;

private:
    QOpenGLShaderProgram *program;
    QOpenGLBuffer vboGrid;
    QOpenGLBuffer vboAxes;
    QOpenGLBuffer vboLine;
    QOpenGLVertexArrayObject vao;
    std::vector<float> plotData;
    std::vector<float> gridData;
};

#endif // LINECHARTSTRATEGY_H
