#include "chartitem.h"

#include "src/fileloader.h"

#include <QOpenGLFunctions>

#include <QOpenGLFramebufferObject>

#include <QWheelEvent>
#include <QMouseEvent>
#include <QNativeGestureEvent>
#include <QEvent>
#include <QGuiApplication>
#include <QtMath>



ChartItem::ChartItem() {

    setAcceptedMouseButtons(Qt::LeftButton);

    setAcceptHoverEvents(true);



    // Kết nối: khi DataManager báo có dữ liệu mới, đặt cờ thay đổi và gọi update() để vẽ lại

    connect(DataManager::instance(), &DataManager::dataChanged, this, [this](){

        m_dataChanged = true;

        resetViewportFromData();

        m_viewChanged = true;

        update();

    });

}



void ChartItem::resetViewportFromData()

{

    auto dm = DataManager::instance();

    const auto data = dm->getData();

    if (data.empty()) {

        m_viewport.resetToDataBounds(0.0f, 1.0f, 0.0f, 1.0f);

        return;

    }

    m_viewport.resetToDataBounds(dm->minX(), dm->maxX(), dm->minY(), dm->maxY());

}



void ChartItem::setChartType(int type)

{

    if(m_chartType!=type){

        m_chartType=type;

        emit chartTypeChanged();

        update();

    }

}



void ChartItem::setChartColor(const QColor &color)

{

    if(m_chartColor!=color)

    {

        m_chartColor=color;

        emit chartColorChanged();

        update();

    }

}



bool ChartItem::loadDataFromFile(const QUrl &fileUrl)
{
    const QString path = fileUrl.isLocalFile()
            ? fileUrl.toLocalFile()
            : fileUrl.toString(QUrl::PreferLocalFile);
    return FileLoader::loadDataset(path);
}



void ChartItem::clearChart()

{

    DataManager::instance()->clear();

}



void ChartItem::resetZoom()

{

    if (!supportsViewportInteraction())

        return;

    resetViewportFromData();

    m_viewChanged = true;

    update();

}



ViewportManager::ZoomAxis ChartItem::zoomAxisFromModifiers(Qt::KeyboardModifiers mods) const
{
    const bool shift = mods.testFlag(Qt::ShiftModifier);
    const bool ctrl = mods.testFlag(Qt::ControlModifier);

    if (shift && !ctrl)
        return ViewportManager::ZoomX;
    if (ctrl && !shift)
        return ViewportManager::ZoomY;
    return ViewportManager::ZoomBoth;
}

void ChartItem::applyZoomAt(float factor, const QPointF &pos, Qt::KeyboardModifiers mods)
{
    if (!supportsViewportInteraction() || factor <= 0.0f)
        return;
    if (DataManager::instance()->getData().empty())
        return;
    if (qAbs(factor - 1.0f) < 1e-5f)
        return;

    float anchorX = 0.0f;
    float anchorY = 0.0f;
    m_viewport.pixelToData(pos.x(), pos.y(), width(), height(), anchorX, anchorY);
    m_viewport.zoom(factor, anchorX, anchorY, zoomAxisFromModifiers(mods));
    m_viewChanged = true;
    update();
}

float ChartItem::zoomFactorFromWheel(const QWheelEvent *event) const
{
    const QPoint pixelDelta = event->pixelDelta();
    const QPoint angleDelta = event->angleDelta();

    // Touchpad (cuộn mượt): ưu tiên pixelDelta
    if (!pixelDelta.isNull()) {
        return qPow(1.002f, -static_cast<float>(pixelDelta.y()));
    }

    // Chuột có con lăn hoặc touchpad gửi angleDelta theo nấc 120
    if (angleDelta.y() != 0) {
        const float steps = angleDelta.y() / 120.0f;
        return qPow(1.15f, steps);
    }

    // Một số touchpad gửi tín hiệu theo trục X
    if (angleDelta.x() != 0) {
        const float steps = angleDelta.x() / 120.0f;
        return qPow(1.15f, steps);
    }

    return 1.0f;
}

bool ChartItem::event(QEvent *event)
{
    if (event->type() == QEvent::NativeGesture && supportsViewportInteraction()) {
        auto *gesture = static_cast<QNativeGestureEvent *>(event);
        if (gesture->gestureType() == Qt::ZoomNativeGesture) {
            if (DataManager::instance()->getData().empty())
                return QQuickFramebufferObject::event(event);

            const double zoomDelta = gesture->value();
            if (!qFuzzyIsNull(zoomDelta)) {
                applyZoomAt(static_cast<float>(1.0 + zoomDelta), gesture->position(),
                            QGuiApplication::keyboardModifiers());
            }
            event->accept();
            return true;
        }
    }
    return QQuickFramebufferObject::event(event);
}

void ChartItem::wheelEvent(QWheelEvent *event)
{
    if (!supportsViewportInteraction()) {
        QQuickFramebufferObject::wheelEvent(event);
        return;
    }

    if (DataManager::instance()->getData().empty()) {
        event->ignore();
        return;
    }

    float factor = zoomFactorFromWheel(event);
    if (event->inverted())
        factor = 1.0f / factor;

    if (qAbs(factor - 1.0f) < 1e-5f) {
        event->ignore();
        return;
    }

    applyZoomAt(factor, event->position(), event->modifiers());
    event->accept();
}



void ChartItem::mousePressEvent(QMouseEvent *event)

{

    if (!supportsViewportInteraction() || event->button() != Qt::LeftButton) {

        QQuickFramebufferObject::mousePressEvent(event);

        return;

    }



    if (DataManager::instance()->getData().empty()) {

        event->ignore();

        return;

    }



    m_panning = true;

    m_lastPanPos = event->position();

    event->accept();

}



void ChartItem::mouseMoveEvent(QMouseEvent *event)

{

    if (!m_panning || !supportsViewportInteraction()) {

        QQuickFramebufferObject::mouseMoveEvent(event);

        return;

    }



    const QPointF delta = event->position() - m_lastPanPos;

    m_lastPanPos = event->position();



    m_viewport.panPixels(static_cast<float>(delta.x()),

                         static_cast<float>(delta.y()),

                         width(), height());

    m_viewChanged = true;

    update();

    event->accept();

}



void ChartItem::mouseReleaseEvent(QMouseEvent *event)

{

    if (event->button() == Qt::LeftButton)

        m_panning = false;

    QQuickFramebufferObject::mouseReleaseEvent(event);

}



void ChartRenderer::synchronize(QQuickFramebufferObject *item)

{

    ChartItem *view = static_cast<ChartItem*>(item);



    // Gửi loại biểu đồ (0, 1, 2...) từ Item xuống Renderer

    m_type = view->chartType();

    m_color = view->chartColor();



    // ĐỒNG BỘ HÓA AN TOÀN ĐA LUỒNG:

    // Copy dữ liệu khi GUI Thread đang bị block tạm thời trong synchronize().

    // DataManager sử dụng std::mutex bên trong nên việc copy tại đây là tuyệt đối an toàn.

    if (view->m_dataChanged || m_renderData.empty() || m_type != m_currentType) {

        auto dm = DataManager::instance();

        m_renderData = dm->getData();

        m_dataMinX = dm->minX();

        m_dataMaxX = dm->maxX();

        m_dataMinY = dm->minY();

        m_dataMaxY = dm->maxY();

        m_dataDirty = true; // Đánh dấu dữ liệu bẩn để nạp VBO

        view->m_dataChanged = false;

    }



    // Đồng bộ viewport (zoom/pan) xuống render thread

    m_viewMinX = view->m_viewport.viewMinX();

    m_viewMaxX = view->m_viewport.viewMaxX();

    m_viewMinY = view->m_viewport.viewMinY();

    m_viewMaxY = view->m_viewport.viewMaxY();

    view->m_viewChanged = false;

}



// khi QML cần hiện một cái gì đó thì

//hàm này sẽ được gọi tự động để sinh ra hàm ChartRenderer

QQuickFramebufferObject::Renderer *ChartItem::createRenderer() const

{

    return new ChartRenderer();

}



//Hàm này được gọi đến và nhảy đến hàm render()

// đóng vai trò như hàm initializeGL()

ChartRenderer::ChartRenderer()

{

    strategy=nullptr;

}



ChartRenderer::~ChartRenderer()

{

    if (strategy) {

        delete strategy;

        strategy = nullptr;

    }

}



//đây là hàm chính để code OpenGL vẽ các biểu đồ

void ChartRenderer::render()

{

    QOpenGLFunctions *f =QOpenGLContext::currentContext()->functions();

    f->glClearColor(0.2f, 0.3f, 0.4f, 1.0f);

    f->glClear(GL_COLOR_BUFFER_BIT);



    // nếu biểu đồ được chọn khác với biểu đồ hiện tại

    if (m_type != m_currentType || strategy == nullptr) {

        // Xóa chiến thuật cũ để tránh tốn RAM

        if (strategy)

        {

            delete strategy;

            strategy=nullptr;

        }



        // Factory Pattern đơn giản ở đây:

        if (m_type == 0) {

            strategy = new LineChartStrategy();

        } else if (m_type == 1) {

            strategy = new BarChartStrategy();  

        }

        else if(m_type==2)

        {

            strategy=new pieChartStrategy();

        }



        if(strategy!=nullptr) strategy->init(); // khởi tạo Shader/VBO cho biểu đồ mới

        m_currentType = m_type; // ghi nhớ loại biểu đồ hiện tại

        m_dataDirty = true; // Strategy mới yêu cầu cập nhật lại VBO

    }



    // 2. THỰC HIỆN VẼ

    // Line/Bar: dùng viewport bounds → shader chỉ vẽ phần đang zoom/pan

    // Pie: dùng data bounds gốc (pie không áp dụng viewport)

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

        strategy->draw(f, time, m_color, m_renderData,

                       drawMinX, drawMaxX, drawMinY, drawMaxY, m_dataDirty);

    }

    m_dataDirty = false; // Reset cờ bẩn sau khi vẽ

}

// kết quả vẽ được dán vào bộ đệm này và hiện ra màn hình

QOpenGLFramebufferObject *ChartRenderer::createFramebufferObject(const QSize &size)

{

    return new QOpenGLFramebufferObject(size);

}


