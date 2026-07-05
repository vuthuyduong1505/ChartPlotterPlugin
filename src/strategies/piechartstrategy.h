#ifndef PIECHARTSTRATEGY_H
#define PIECHARTSTRATEGY_H

#include "src/chartstrategy.h"
#include "src/datamanager.h"
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <vector>
#include <QString>

struct PieSliceInfo {
    float weight;
    QString name;
    float startAngle;
    float arcAngle;
    QVector4D color;
    float binMin;
    float binMax;
};

class pieChartStrategy : public ChartStrategy
{
public:
    pieChartStrategy();
    ~pieChartStrategy() override;
    void init() override;
    void draw(QOpenGLFunctions *f, float time, const QColor &color,
              const std::vector<DataPoint> &rawData,
              float minX, float maxX, float minY, float maxY,
              bool dataDirty, int lineStyle = 0) override;

    void setHoveredSlice(int slice) override { m_hoveredSlice = slice; }
    void setPieBinMode(int mode) override { m_pieBinMode = mode; }

    static std::vector<PieSliceInfo> computeSlices(const std::vector<DataPoint> &rawData,
                                                   float minX, float maxX, float minY, float maxY,
                                                   int binMode, const QColor &baseColor);

private:
    QOpenGLBuffer vboQuad;
    QOpenGLShaderProgram *program;
    QOpenGLVertexArrayObject vao;
    int m_hoveredSlice = -1;
    int m_pieBinMode = 0;
};

#endif // PIECHARTSTRATEGY_H
