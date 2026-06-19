#ifndef PIECHARTSTRATEGY_H
#define PIECHARTSTRATEGY_H

#include "chartstrategy.h"
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <vector>
class pieChartStrategy : public ChartStrategy
{
public:
    pieChartStrategy();
    ~pieChartStrategy();
    void init() override;
    void draw(QOpenGLFunctions *f, float time) override;

private:
    QOpenGLBuffer vboPie;
    QOpenGLShaderProgram *program;
    QOpenGLVertexArrayObject vao;
};

#endif // PIECHARTSTRATEGY_H
