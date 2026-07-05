#include "chartitem.h"
#include "chartrenderer.h"
#include "dataprocessor.h"

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
    QVariantMap result;
    result["valid"] = false;
    result["dataX"] = 0.0f;
    result["dataY"] = 0.0f;
    result["screenX"] = 0.0f;
    result["screenY"] = 0.0f;
    result["isPie"] = false;
    result["percent"] = 0.0f;

    if (screenWidth <= 0.0f || screenHeight <= 0.0f) {
        return result;
    }

    auto dm = DataManager::instance();
    if (dm->isEmpty()) {
        return result;
    }

    // Bước 1: Ánh xạ ngược mouseX sang giá trị targetDataX
    float xGL = (mouseX / screenWidth) * 2.0f - 1.0f;
    float mapMinX = -1.0f;
    float mapMaxX = 1.0f;
    float mapMinY = -1.0f;
    float mapMaxY = 1.0f;

    float u_minX = m_viewport.viewMinX();
    float u_maxX = m_viewport.viewMaxX();

    float denX = (u_maxX - u_minX) == 0.0f ? 0.001f : (u_maxX - u_minX);
    float targetDataX = u_minX + (xGL - mapMinX) * denX / (mapMaxX - mapMinX);

    // Ràng buộc targetDataX trong khoảng dữ liệu thực tế hiện có để tránh ngoại suy lỗi
    targetDataX = std::max(dm->firstPoint().x, std::min(dm->lastPoint().x, targetDataX));

    DataPoint p1, p2;
    if (m_chartType != 2) {
        std::pair<DataPoint, DataPoint> adj = dm->findAdjacentPoints(targetDataX);
        p1 = adj.first;
        p2 = adj.second;
    }

    if (m_chartType == 0) {
        // --- LINE CHART: Nội suy tuyến tính Y ---
        float interpolatedDataY = p1.y;
        if (p2.x - p1.x != 0.0f) {
            interpolatedDataY = p1.y + (p2.y - p1.y) * ((targetDataX - p1.x) / (p2.x - p1.x));
        }

        // Ánh xạ xuôi (Forward Mapping) để chuyển (targetDataX, interpolatedDataY) về pixel màn hình
        float snappedXGL = (mapMaxX - mapMinX) * (targetDataX - u_minX) / denX + mapMinX;

        float u_minY = m_viewport.viewMinY();
        float u_maxY = m_viewport.viewMaxY();
        if (qAbs(u_maxY - u_minY) < 0.0001f) {
            u_maxY += 1.0f;
            u_minY -= 1.0f;
        }

        float denY = (u_maxY - u_minY) == 0.0f ? 0.001f : (u_maxY - u_minY);
        float snappedYGL = (mapMaxY - mapMinY) * (interpolatedDataY - u_minY) / denY + mapMinY;

        float lineScreenY = (1.0f - snappedYGL) * 0.5f * screenHeight;

        float distanceY = qAbs(mouseY - lineScreenY);

        if (distanceY <= 15.0f) {
            result["valid"] = true;
            result["dataX"] = targetDataX;
            result["dataY"] = interpolatedDataY;
            result["screenX"] = mouseX; // Bám trơn tru theo con trỏ chuột
            result["screenY"] = lineScreenY; // Hút thẳng vào nét vẽ
        } else {
            result["valid"] = false;
        }
    } else if (m_chartType == 1) {
        // --- BAR CHART: PIXEL-PERFECT HIT TESTING ---
        float u_minY = m_viewport.viewMinY();
        float u_maxY = m_viewport.viewMaxY();
        if (qAbs(u_maxY - u_minY) < 0.0001f) {
            u_maxY += 1.0f;
            u_minY -= 1.0f;
        }
        float denY = (u_maxY - u_minY) == 0.0f ? 0.001f : (u_maxY - u_minY);

        // 1. Tính bề rộng chuẩn của cột trên màn hình pixel (Đồng bộ tuyệt đối với shader: 0.015f * mapRangeX)
        float mapRangeX = mapMaxX - mapMinX; 
        float pixelHalfWidth = mapRangeX * 0.015f * 0.5f * screenWidth;
        
        // 2. Tính tọa độ Y của đáy cột (zeroScreenY)
        float zeroYGL = (mapMaxY - mapMinY) * (0.0f - u_minY) / denY + mapMinY;
        float zeroScreenY = (1.0f - zeroYGL) * 0.5f * screenHeight;

        // 3. Hàm Lambda phụ trợ: Ánh xạ 1 điểm ra màn hình và kiểm tra chuột có nằm trong hình chữ nhật của nó không
        auto checkHit = [&](const DataPoint& p, float& screenX, float& screenY) -> bool {
            // Ánh xạ X
            float pXGL = (mapMaxX - mapMinX) * (p.x - u_minX) / denX + mapMinX;
            screenX = (pXGL + 1.0f) * 0.5f * screenWidth;
            // Ánh xạ Y
            float pYGL = (mapMaxY - mapMinY) * (p.y - u_minY) / denY + mapMinY;
            screenY = (1.0f - pYGL) * 0.5f * screenHeight;
            
            // Kiểm tra chuột có nằm trong vùng Pixel Rectangle của cột (mở rộng 1px chống nhiễu)
            bool withinX = (mouseX >= screenX - pixelHalfWidth - 1.0f) && (mouseX <= screenX + pixelHalfWidth + 1.0f);
            float minYBound = qMin(screenY, zeroScreenY);
            float maxYBound = qMax(screenY, zeroScreenY);
            bool withinY = (mouseY >= minYBound) && (mouseY <= maxYBound);
            
            return withinX && withinY;
        };

        // 4. Áp dụng kiểm tra trực tiếp Hitbox cho cả 2 điểm p1 và p2
        float sX1 = 0, sY1 = 0, sX2 = 0, sY2 = 0;
        bool hit1 = checkHit(p1, sX1, sY1);
        bool hit2 = checkHit(p2, sX2, sY2);

        if (hit1) {
            result["valid"] = true;
            result["dataX"] = p1.x;
            result["dataY"] = p1.y;
            result["screenX"] = sX1;
            result["screenY"] = sY1;
        } else if (hit2) {
            result["valid"] = true;
            result["dataX"] = p2.x;
            result["dataY"] = p2.y;
            result["screenX"] = sX2;
            result["screenY"] = sY2;
        } else {
            result["valid"] = false;
        }
    } else if (m_chartType == 2) {
        // --- PIE CHART: HIT-TESTING ĐỒNG BỘ VỚI OPENGL ---
        float cx = screenWidth / 2.0f;
        float cy = screenHeight / 2.0f;
        // Bán kính 0.6f theo chuẩn code PieChartStrategy
        float radius = 0.6f * (qMin(screenWidth, screenHeight) / 2.0f); 

        float dx = mouseX - cx;
        // QUAN TRỌNG: Lật ngược trục Y (cy - mouseY) để đồng bộ với hàm sin/cos của OpenGL
        float dy = cy - mouseY; 
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist <= radius) {
            // Đồng bộ thuật toán computeSliceWeights của pieChartStrategy
            struct SliceInfo {
                float weight;
                float dataX;
                float dataY;
            };
            std::vector<SliceInfo> slices;

            const auto data = dm->getData();
            float minX = data.front().x;
            float maxX = data.back().x;
            constexpr int kMaxDirectSlices = 12;
            constexpr int kBinSliceCount = 8;

            if (data.size() <= kMaxDirectSlices) {
                for (const auto& p : data) {
                    float w = qMax(p.y, 0.0f);
                    if (w > 0.0f) {
                        slices.push_back({w, p.x, p.y});
                    }
                }
            } else {
                std::vector<float> binWeights(kBinSliceCount, 0.0f);
                std::vector<float> binXSum(kBinSliceCount, 0.0f);
                std::vector<int> binCount(kBinSliceCount, 0);
                
                float rangeX = maxX - minX;
                if (rangeX < 1e-6f) rangeX = 1.0f;

                for (const auto& p : data) {
                    int bin = static_cast<int>((p.x - minX) / rangeX * kBinSliceCount);
                    bin = std::min(std::max(bin, 0), kBinSliceCount - 1);
                    float w = qMax(p.y, 0.0f);
                    binWeights[bin] += w;
                    binXSum[bin] += p.x;
                    binCount[bin]++;
                }

                for (int i = 0; i < kBinSliceCount; ++i) {
                    if (binWeights[i] > 0.0f) {
                        float avgX = binCount[i] > 0 ? (binXSum[i] / binCount[i]) : 0.0f;
                        slices.push_back({binWeights[i], avgX, binWeights[i]});
                    }
                }
            }

            float totalSum = 0.0f;
            for (const auto& s : slices) {
                totalSum += s.weight;
            }

            if (totalSum > 0.0f) {
                float mouseAngle = std::atan2(dy, dx) * 180.0f / M_PI;
                if (mouseAngle < 0.0f) mouseAngle += 360.0f;

                float currentAngle = 0.0f;
                for (const auto& s : slices) {
                    float sliceAngle = (s.weight / totalSum) * 360.0f;
                    
                    if (sliceAngle > 0.0f && mouseAngle >= currentAngle && mouseAngle < currentAngle + sliceAngle) {
                        result["valid"] = true;
                        result["isPie"] = true; // Bật cờ Pie Chart
                        result["percent"] = (s.weight / totalSum) * 100.0f; // Tính %
                        result["dataX"] = s.dataX;
                        result["dataY"] = s.dataY;
                        result["screenX"] = mouseX; 
                        result["screenY"] = mouseY;
                        break;
                    }
                    currentAngle += sliceAngle;
                }
            }
        }
    }

    return result;
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

float ChartItem::calculateGridStep(float range) const
{
    float rawStep = range / 8.0f;
    if (rawStep <= 0.0f) return 1.0f;
    float logStep = std::log10(rawStep);
    float exponent = std::floor(logStep);
    float base = std::pow(10.0f, exponent);
    float fraction = rawStep / base;

    float step;
    if (fraction < 1.5f) step = 1.0f * base;
    else if (fraction < 3.0f) step = 2.0f * base;
    else if (fraction < 7.0f) step = 5.0f * base;
    else step = 10.0f * base;
    return step;
}

QVariantList ChartItem::calculateTicks(float minVal, float maxVal)
{
    QVariantList list;
    float range = maxVal - minVal;
    if (range <= 0.0f) return list;

    float step = calculateGridStep(range);

    // Tìm tick bắt đầu lớn hơn hoặc bằng minVal và chia hết cho step
    float firstTick = std::ceil(minVal / step) * step;

    for (float val = firstTick; val <= maxVal; val += step) {
        if (val < minVal - 1e-5f || val > maxVal + 1e-5f) continue;

        QVariantMap tick;
        tick["value"] = QString::number(val, 'g', 6);
        tick["position"] = (val - minVal) / range;
        tick["val"] = val;
        list.append(tick);
    }
    return list;
}

QVariantList ChartItem::xTicks() const
{
    return const_cast<ChartItem*>(this)->calculateTicks(m_viewport.viewMinX(), m_viewport.viewMaxX());
}

QVariantList ChartItem::yTicks() const
{
    return const_cast<ChartItem*>(this)->calculateTicks(m_viewport.viewMinY(), m_viewport.viewMaxY());
}


