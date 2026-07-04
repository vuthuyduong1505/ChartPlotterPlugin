#ifndef CHARTRENDERER_H
#define CHARTRENDERER_H

#include <QQuickFramebufferObject>
#include <QOpenGLFunctions>
#include <QColor>
#include <vector>
#include "src/datamanager.h"
#include "src/chartstrategy.h"

class ChartItem;

class ChartRenderer : public QQuickFramebufferObject::Renderer, protected QOpenGLFunctions
{
public:
    ChartRenderer();
    ~ChartRenderer() override;

    void render() override;
    void synchronize(QQuickFramebufferObject *item) override;
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;

private:
    ChartStrategy *strategy = nullptr;
    float time = 0.0f;
    int m_type = 0;
    int m_currentType = -1;
    QColor m_color;
    int m_lineStyle = 0;

    std::vector<DataPoint> m_renderData;
    float m_dataMinX = 0.0f;
    float m_dataMaxX = 1.0f;
    float m_dataMinY = 0.0f;
    float m_dataMaxY = 1.0f;

    float m_viewMinX = 0.0f;
    float m_viewMaxX = 1.0f;
    float m_viewMinY = 0.0f;
    float m_viewMaxY = 1.0f;
    bool m_dataDirty = false;
};

#endif // CHARTRENDERER_H
