#include "chartitem.h"
#include "chartrenderer.h"
#include "dataprocessor.h"
#include "charthittester.h"
#include "chartaxiscalculator.h"

#include "src/fileloader.h"
#include <QDebug>

#include <QOpenGLFunctions>

#include <QOpenGLFramebufferObject>

#include <QWheelEvent>
#include <QMouseEvent>
#include <QNativeGestureEvent>
#include <QEvent>
#include <QGuiApplication>
#include <QtMath>
#include <algorithm>



ChartItem::ChartItem() {
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptHoverEvents(true);

    // Kết nối: khi DataManager báo có dữ liệu mới, đặt cờ thay đổi và gọi update() để vẽ lại
    connect(DataManager::instance(), &DataManager::dataChanged, this, [this](){
        m_dataChanged = true;
        updateAutoPanLogic();
        m_viewChanged = true;
        emit xTicksChanged();
        emit yTicksChanged();
        update();
    });

    // Kết nối cập nhật Ticks khi viewport dịch chuyển hoặc co giãn
    connect(this, &ChartItem::zoomXChanged, this, &ChartItem::xTicksChanged);
    connect(this, &ChartItem::panXChanged, this, &ChartItem::xTicksChanged);
    connect(this, &ChartItem::zoomYChanged, this, &ChartItem::yTicksChanged);
    connect(this, &ChartItem::panYChanged, this, &ChartItem::yTicksChanged);
}



void ChartItem::resetViewportFromData()
{
    const auto data = DataManager::instance()->getData();
    if (data.empty()) {
        m_viewport.resetToDataBounds(0.0f, 1.0f, 0.0f, 1.0f);
        emit zoomXChanged();
        emit zoomYChanged();
        emit panXChanged();
        emit panYChanged();
        emit dataBoundsChanged();
        return;
    }
    float minX, maxX, minY, maxY;
    DataProcessor::calculateBounds(data, minX, maxX, minY, maxY);
    // Tự động tính toán khoảng xem ngang hiển thị 10.000 điểm dữ liệu đầu tiên
    int dataSize = static_cast<int>(data.size());
    float spanX = (maxX - minX);
    if (dataSize > 10000) {
        spanX = (maxX - minX) * (10000.0f / dataSize);
    }

    // Không khoảng đệm (Tight Fit): điểm đầu tiên khớp lề trái (-1.0 trong OpenGL)
    float viewMinX = minX;
    float viewMaxX = minX + spanX;

    // Tính giá trị Y trung bình (avgY) của dữ liệu thô
    float sumY = 0.0f;
    for (const auto& p : data) {
        sumY += p.y;
    }
    float avgY = sumY / dataSize;

    // Căn dọc khít chặt (Tight Fit) căn giữa theo avgY
    float halfRangeY = std::max(maxY - avgY, avgY - minY);
    if (halfRangeY < 1e-4f) halfRangeY = 0.5f;
    float viewMinY = avgY - halfRangeY;
    float viewMaxY = avgY + halfRangeY;

    // Cài đặt biên dữ liệu gốc và thiết lập viewport ban đầu
    m_viewport.resetToDataBounds(minX, maxX, minY, maxY);
    m_viewport.setViewBoundsX(viewMinX, viewMaxX);
    m_viewport.setViewBoundsY(viewMinY, viewMaxY);

    emit zoomXChanged();
    emit zoomYChanged();
    emit panXChanged();
    emit panYChanged();
    emit dataBoundsChanged();
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

void ChartItem::setPieBinMode(int mode)
{
    if (m_pieBinMode != mode) {
        m_pieBinMode = mode;
        emit pieBinModeChanged();
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
    emit zoomXChanged();
    emit zoomYChanged();
    emit panXChanged();
    emit panYChanged();
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

    if (zoomX() <= 1.001f && zoomY() <= 1.001f) {

        QQuickFramebufferObject::mousePressEvent(event);

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
    if (zoomX() <= 1.001f && zoomY() <= 1.001f) {
        return;
    }
    m_viewport.panPixels(static_cast<float>(delta.x()),
                         static_cast<float>(delta.y()),
                         width(), height());
    m_viewChanged = true;
    emit panXChanged();
    emit panYChanged();
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
            emit zoomXChanged();
            emit zoomYChanged();
            emit panXChanged();
            emit panYChanged();
            emit dataBoundsChanged();
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

QVariantMap ChartItem::getNearestDataPoint(float mouseX, float mouseY, float screenWidth, float screenHeight)
{
    bool sliceChanged = false;
    QVariantMap res = ChartHitTester::getNearestDataPoint(mouseX, mouseY, screenWidth, screenHeight,
                                                          m_chartType, m_pieBinMode, m_chartColor,
                                                          m_viewport, m_hoveredSlice, sliceChanged);
    if (sliceChanged) {
        update();
    }
    return res;
}

float ChartItem::zoomX() const {
    float dataRange = m_viewport.dataMaxX() - m_viewport.dataMinX();
    float viewRange = m_viewport.viewMaxX() - m_viewport.viewMinX();
    return viewRange > 0.0f ? (dataRange / viewRange) : 1.0f;
}
void ChartItem::setZoomX(float val) {
    if (val <= 0.0f) return;
    float dataRange = m_viewport.dataMaxX() - m_viewport.dataMinX();
    if (dataRange <= 0.0f) dataRange = 1.0f;
    float newRange = dataRange / val;
    m_viewport.setViewBoundsX(m_viewport.viewMinX(), m_viewport.viewMinX() + newRange);
    m_viewChanged = true;
    emit zoomXChanged();
    update();
}
float ChartItem::zoomY() const {
    float dataRange = m_viewport.dataMaxY() - m_viewport.dataMinY();
    float viewRange = m_viewport.viewMaxY() - m_viewport.viewMinY();
    return viewRange > 0.0f ? (dataRange / viewRange) : 1.0f;
}
void ChartItem::setZoomY(float val) {
    if (val <= 0.0f) return;
    float dataRange = m_viewport.dataMaxY() - m_viewport.dataMinY();
    if (dataRange <= 0.0f) dataRange = 1.0f;
    float newRange = dataRange / val;
    m_viewport.setViewBoundsY(m_viewport.viewMinY(), m_viewport.viewMinY() + newRange);
    m_viewChanged = true;
    emit zoomYChanged();
    update();
}
float ChartItem::panX() const {
    return m_viewport.viewMinX();
}
void ChartItem::setPanX(float val) {
    if (zoomX() <= 1.001f && zoomY() <= 1.001f) return;
    float viewRange = m_viewport.viewMaxX() - m_viewport.viewMinX();
    m_viewport.setViewBoundsX(val, val + viewRange);
    m_viewChanged = true;
    emit panXChanged();
    update();
}
float ChartItem::panY() const {
    return m_viewport.viewMinY();
}
void ChartItem::setPanY(float val) {
    if (zoomX() <= 1.001f && zoomY() <= 1.001f) return;
    float viewRange = m_viewport.viewMaxY() - m_viewport.viewMinY();
    m_viewport.setViewBoundsY(val, val + viewRange);
    m_viewChanged = true;
    emit panYChanged();
    update();
}
float ChartItem::dataMinX() const { return m_viewport.dataMinX(); }
float ChartItem::dataMaxX() const { return m_viewport.dataMaxX(); }
float ChartItem::dataMinY() const { return m_viewport.dataMinY(); }
float ChartItem::dataMaxY() const { return m_viewport.dataMaxY(); }

void ChartItem::createStressTestData()
{
    DataManager::instance()->createStressTestData();
    resetZoom();
}

QVariantList ChartItem::xTicks() const
{
    return ChartAxisCalculator::calculateTicks(m_viewport.viewMinX(), m_viewport.viewMaxX());
}

QVariantList ChartItem::yTicks() const
{
    return ChartAxisCalculator::calculateTicks(m_viewport.viewMinY(), m_viewport.viewMaxY());
}


