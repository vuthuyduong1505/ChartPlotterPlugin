#include "chartrenderer.h"
#include "chartitem.h"
#include "dataprocessor.h"
#include "strategies/chartstrategyfactory.h"
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>

ChartRenderer::ChartRenderer()
{
    strategy = nullptr;
}

ChartRenderer::~ChartRenderer()
{
    if (strategy) {
        delete strategy;
        strategy = nullptr;
    }
}

void ChartRenderer::synchronize(QQuickFramebufferObject *item)
{
    ChartItem *view = static_cast<ChartItem*>(item);

    m_type = view->chartType();
    m_color = view->chartColor();
    m_lineStyle = view->lineStyle();
    m_hoveredSlice = view->m_hoveredSlice;
    m_pieBinMode = view->m_pieBinMode;

    // ĐỒNG BỘ HÓA AN TOÀN ĐA LUỒNG
    if (view->m_dataChanged || m_renderData.empty() || m_type != m_currentType) {
        auto dm = DataManager::instance();
        m_renderData = dm->getData();
        if (m_renderData.empty()) {
            m_dataMinX = 0.0f;
            m_dataMaxX = 1.0f;
            m_dataMinY = 0.0f;
            m_dataMaxY = 1.0f;
        } else {
            // Tính toán biên dữ liệu dựa trên tập dữ liệu GỐC để đảm bảo lưới tọa độ chính xác
            DataProcessor::calculateBounds(m_renderData, m_dataMinX, m_dataMaxX, m_dataMinY, m_dataMaxY);
            // Thực hiện giảm mẫu bằng LTTB nếu dữ liệu vẽ lớn hơn 50000 điểm
            if (m_renderData.size() > 50000) {
                m_renderData = DataProcessor::downsampleLTTB(m_renderData, 50000);
            }
        }
        m_dataDirty = true;
        view->m_dataChanged = false;
    }

    m_viewMinX = view->m_viewport.viewMinX();
    m_viewMaxX = view->m_viewport.viewMaxX();
    m_viewMinY = view->m_viewport.viewMinY();
    m_viewMaxY = view->m_viewport.viewMaxY();
    view->m_viewChanged = false;
}

void ChartRenderer::render()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT);

    if (m_type != m_currentType || strategy == nullptr) {
        if (strategy) {
            delete strategy;
            strategy = nullptr;
        }

        // Sử dụng Factory Pattern chuẩn hóa
        strategy = ChartStrategyFactory::createStrategy(m_type);

        if (strategy != nullptr) strategy->init();
        m_currentType = m_type;
        m_dataDirty = true;
    }

    if (strategy) {
        float drawMinX = m_viewMinX;
        float drawMaxX = m_viewMaxX;
        float drawMinY = m_viewMinY;
        float drawMaxY = m_viewMaxY;

        if (m_type == 2) {
            drawMinX = m_dataMinX;
            drawMaxX = m_dataMaxX;
            drawMinY = m_dataMinY;
            drawMaxY = m_dataMaxY;
        }

        strategy->setHoveredSlice(m_hoveredSlice);
        strategy->setPieBinMode(m_pieBinMode);
        strategy->draw(f, time, m_color, m_renderData,
                       drawMinX, drawMaxX, drawMinY, drawMaxY, m_dataDirty, m_lineStyle);
    }
    m_dataDirty = false;
}

QOpenGLFramebufferObject *ChartRenderer::createFramebufferObject(const QSize &size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setSamples(4);
    return new QOpenGLFramebufferObject(size, format);
}
