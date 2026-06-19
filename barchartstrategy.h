#ifndef BARCHARTSTRATEGY_H
#define BARCHARTSTRATEGY_H

#include "chartstrategy.h"
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include<vector>
class BarChartStrategy : public ChartStrategy
{
public:
    BarChartStrategy();
    ~BarChartStrategy();
    void init() override;
    void draw(QOpenGLFunctions *f, float time) override;
private:
    QOpenGLShaderProgram *program;
    QOpenGLBuffer vboBar;
    QOpenGLVertexArrayObject vao;
    std::vector<float> barData;
    float mapvalue(float input, float inputMin, float inputMax, float outputMin, float outputMax);


};

#endif // BARCHARTSTRATEGY_H
