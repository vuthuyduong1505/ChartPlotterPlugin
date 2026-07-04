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
    static constexpr int kMaxDirectSlices = 12;
    static constexpr int kBinSliceCount = 8;

    std::vector<float> computeSliceWeights(const std::vector<DataPoint> &rawData,
                                           float minX, float maxX) const;
    void rebuildSliceGeometry(QOpenGLFunctions *f, float scaleX, float scaleY);
    QVector4D sliceColor(const QColor &base, int index, int total) const;

    std::vector<QOpenGLBuffer> vboSlices;
    std::vector<int> m_vertexCounts;
    std::vector<float> m_sliceWeights;
    QOpenGLShaderProgram *program;
    QOpenGLVertexArrayObject vao;
    int m_lastViewportW = 0;
    int m_lastViewportH = 0;
};

#endif // PIECHARTSTRATEGY_H
