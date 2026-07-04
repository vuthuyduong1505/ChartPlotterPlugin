#include "chartitem.h"
#include "chartrenderer.h"
#include "dataprocessor.h"

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
        updateAutoPanLogic();
        m_viewChanged = true;
        update();
    });

}



void ChartItem::resetViewportFromData()
{
    const auto data = DataManager::instance()->getData();
    if (data.empty()) {
        m_viewport.resetToDataBounds(0.0f, 1.0f, 0.0f, 1.0f);
        return;
    }
    float minX, maxX, minY, maxY;
    DataProcessor::calculateBounds(data, minX, maxX, minY, maxY);
    m_viewport.resetToDataBounds(minX, maxX, minY, maxY);
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



void ChartItem::setDataMode(int mode)
{
    mode = qBound(0, mode, 1);
    if (m_dataMode == mode)
        return;

    if (mode == 1) {
        OnlineStream::instance()->stop();
        DataManager::instance()->clear();
        resetViewportFromData();
        m_isAutoPanEnabled = true;
        emit isAutoPanEnabledChanged();
        OnlineStream::instance()->start();
    } else {
        OnlineStream::instance()->stop();
    }

    m_dataMode = mode;
    emit dataModeChanged();
    m_viewChanged = true;
    update();
}

void ChartItem::setIsAutoPanEnabled(bool enabled)
{
    if (m_isAutoPanEnabled != enabled) {
        m_isAutoPanEnabled = enabled;
        emit isAutoPanEnabledChanged();
    }
}

void ChartItem::setLineStyle(int style)
{
    if (m_lineStyle != style) {
        m_lineStyle = style;
        emit lineStyleChanged();
        update();
    }
}

void ChartItem::pauseStream()
{
    OnlineStream::instance()->pauseStream();
}

void ChartItem::resumeStream()
{
    OnlineStream::instance()->resumeStream();
}



bool ChartItem::loadDataFromFile(const QUrl &fileUrl)
{
    if (m_dataMode == 1) {
        OnlineStream::instance()->stop();
        m_dataMode = 0;
        emit dataModeChanged();
    }

    const QString path = fileUrl.isLocalFile()
            ? fileUrl.toLocalFile()
            : fileUrl.toString(QUrl::PreferLocalFile);
    return FileLoader::loadDataset(path);
}



void ChartItem::clearChart()
{
    DataManager::instance()->clear();
    if (m_dataMode == 1) {
        OnlineStream::instance()->stop();
        OnlineStream::instance()->start();
    }
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

void ChartItem::processZoom(float factor, const QPointF& center)
{
    if (!supportsViewportInteraction() || factor <= 0.0f)
        return;
    if (DataManager::instance()->getData().empty())
        return;
    if (qAbs(factor - 1.0f) < 1e-5f)
        return;

    float anchorX = 0.0f;
    float anchorY = 0.0f;
    m_viewport.pixelToData(center.x(), center.y(), width(), height(), anchorX, anchorY);
    m_viewport.zoom(factor, anchorX, anchorY, zoomAxisFromModifiers(QGuiApplication::keyboardModifiers()));
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
                processZoom(static_cast<float>(1.0 + zoomDelta), gesture->position());
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

    processZoom(factor, event->position());
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



    processPan(delta);

    event->accept();

}



void ChartItem::mouseReleaseEvent(QMouseEvent *event)

{

    if (event->button() == Qt::LeftButton)

        m_panning = false;

    QQuickFramebufferObject::mouseReleaseEvent(event);

}







void ChartItem::processPan(const QPointF& delta)
{
    m_viewport.panPixels(static_cast<float>(delta.x()),
                         static_cast<float>(delta.y()),
                         width(), height());
    m_viewChanged = true;
    update();
}

void ChartItem::updateAutoPanLogic()
{
    if (m_dataMode == 1 && m_isAutoPanEnabled) {
        auto dm = DataManager::instance();
        const auto data = dm->getData();
        if (!data.empty()) {
            float minX, maxX, minY, maxY;
            DataProcessor::calculateBounds(data, minX, maxX, minY, maxY);

            // Cập nhật lại phạm vi biên dữ liệu
            m_viewport.resetToDataBounds(minX, maxX, minY, maxY);

            float currentMaxX = maxX;
            float FIXED_SPAN_X = 150.0f; // Khớp với FIXED_SPAN_X bên strategy

            float newViewMinX = 0.0f;
            float newViewMaxX = FIXED_SPAN_X;

            if (currentMaxX > FIXED_SPAN_X) {
                // Giai đoạn 2: Đã vượt mép phải -> Kích hoạt Auto-Pan kéo camera lùi lại
                newViewMaxX = currentMaxX;
                newViewMinX = currentMaxX - FIXED_SPAN_X;
            } else {
                // Giai đoạn 1: Chưa đầy màn hình -> Camera đứng im, đồ thị mọc từ trái sang phải
                newViewMinX = 0.0f;
                newViewMaxX = FIXED_SPAN_X;
            }
            m_viewport.setViewBoundsX(newViewMinX, newViewMaxX);
        } else {
            resetViewportFromData();
        }
    } else if (m_dataMode == 0) {
        resetViewportFromData();
    }
}

// khi QML cần hiện một cái gì đó thì
//hàm này sẽ được gọi tự động để sinh ra hàm ChartRenderer
QQuickFramebufferObject::Renderer *ChartItem::createRenderer() const
{
    return new ChartRenderer();
}


