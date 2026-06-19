#ifndef LINECHARTSTRATEGY_H
#define LINECHARTSTRATEGY_H

#include "src/chartstrategy.h"
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include<vector>
class LineChartStrategy : public ChartStrategy
{
public:
    LineChartStrategy();
    ~LineChartStrategy();

    void init() override;
    void draw(QOpenGLFunctions *f, float time) override;

private:
    QOpenGLShaderProgram *program;
    QOpenGLBuffer vboGrid;
    QOpenGLBuffer vboAxes;
    QOpenGLBuffer vboLine;
    QOpenGLVertexArrayObject vao;
    std::vector<float> plotData;
    float mapvalue(float input, float inputMin, float inputMax, float outputMin, float outputMax);
    std::vector<float> gridData;
};

#endif // LINECHARTSTRATEGY_H
