#include "viewportmanager.h"
#include <algorithm>
#include <cmath>

ViewportManager::ViewportManager() = default;

void ViewportManager::resetToDataBounds(float minX, float maxX, float minY, float maxY)
{
    m_dataMinX = minX;
    m_dataMaxX = maxX;
    m_dataMinY = minY;
    m_dataMaxY = maxY;

    m_viewMinX = minX;
    m_viewMaxX = maxX;
    m_viewMinY = minY;
    m_viewMaxY = maxY;

    if (m_viewMaxX - m_viewMinX < 1e-6f) {
        m_viewMaxX = m_viewMinX + 1.0f;
        m_dataMaxX = m_viewMaxX;
    }
    if (m_viewMaxY - m_viewMinY < 1e-6f) {
        m_viewMaxY = m_viewMinY + 1.0f;
        m_dataMaxY = m_viewMaxY;
    }
}

void ViewportManager::setViewBoundsX(float minX, float maxX)
{
    m_viewMinX = minX;
    m_viewMaxX = maxX;
    clampViewport();
}

void ViewportManager::zoomOneAxis(float &viewMin, float &viewMax, float anchor, float factor, bool enabled)
{
    if (!enabled || factor <= 0.0f)
        return;

    const float range = viewMax - viewMin;
    if (range <= 0.0f)
        return;

    const float newRange = range / factor;
    const float rel = (anchor - viewMin) / range; // vị trí neo trong viewport hiện tại [0..1]

    viewMin = anchor - rel * newRange;
    viewMax = viewMin + newRange;
}

void ViewportManager::zoom(float factor, float anchorX, float anchorY, ZoomAxis axis)
{
    zoomOneAxis(m_viewMinX, m_viewMaxX, anchorX, factor, axis == ZoomBoth || axis == ZoomX);
    zoomOneAxis(m_viewMinY, m_viewMaxY, anchorY, factor, axis == ZoomBoth || axis == ZoomY);
    clampViewport();
}

void ViewportManager::panPixels(float dx, float dy, float widgetWidth, float widgetHeight)
{
    if (widgetWidth <= 0.0f || widgetHeight <= 0.0f)
        return;

    const float rangeX = m_viewMaxX - m_viewMinX;
    const float rangeY = m_viewMaxY - m_viewMinY;

    // Kéo sang phải → nội dung đi theo → viewport dịch sang trái trong không gian dữ liệu
    const float deltaX = -(dx / widgetWidth) * rangeX;
    // Kéo xuống → viewport dịch lên theo trục Y dữ liệu (Y lớn ở trên màn hình)
    const float deltaY = (dy / widgetHeight) * rangeY;

    m_viewMinX += deltaX;
    m_viewMaxX += deltaX;
    m_viewMinY += deltaY;
    m_viewMaxY += deltaY;

    clampViewport();
}

void ViewportManager::pixelToData(float px, float py, float widgetWidth, float widgetHeight,
                                  float &outX, float &outY) const
{
    if (widgetWidth <= 0.0f || widgetHeight <= 0.0f) {
        outX = (m_viewMinX + m_viewMaxX) * 0.5f;
        outY = (m_viewMinY + m_viewMaxY) * 0.5f;
        return;
    }

    const float normX = px / widgetWidth;
    const float normY = py / widgetHeight;

    outX = m_viewMinX + normX * (m_viewMaxX - m_viewMinX);
    // Trên màn hình: py=0 là đỉnh → tương ứng viewMaxY
    outY = m_viewMaxY - normY * (m_viewMaxY - m_viewMinY);
}

void ViewportManager::clampViewport()
{
    const float dataRangeX = std::max(m_dataMaxX - m_dataMinX, 1e-6f);
    const float dataRangeY = std::max(m_dataMaxY - m_dataMinY, 1e-6f);

    float rangeX = m_viewMaxX - m_viewMinX;
    float rangeY = m_viewMaxY - m_viewMinY;

    // Không cho zoom in quá sâu (tối thiểu 1% phạm vi dữ liệu gốc)
    rangeX = std::max(rangeX, dataRangeX * 0.01f);
    rangeY = std::max(rangeY, dataRangeY * 0.01f);

    // Không cho zoom out rộng hơn 4 lần dữ liệu gốc (tối thiểu là 150.0f)
    float maxRangeX = std::max(dataRangeX * 4.0f, 150.0f);
    rangeX = std::min(rangeX, maxRangeX);
    rangeY = std::min(rangeY, dataRangeY * 4.0f);

    const float centerX = (m_viewMinX + m_viewMaxX) * 0.5f;
    const float centerY = (m_viewMinY + m_viewMaxY) * 0.5f;

    m_viewMinX = centerX - rangeX * 0.5f;
    m_viewMaxX = centerX + rangeX * 0.5f;
    m_viewMinY = centerY - rangeY * 0.5f;
    m_viewMaxY = centerY + rangeY * 0.5f;

    // Giữ viewport không trôi quá xa khỏi vùng dữ liệu (padding 50% mỗi phía, tối thiểu 150.0f)
    const float padX = std::max(dataRangeX * 0.5f, 150.0f);
    const float padY = dataRangeY * 0.5f;
    const float limitMinX = m_dataMinX - padX;
    const float limitMaxX = m_dataMaxX + padX;
    const float limitMinY = m_dataMinY - padY;
    const float limitMaxY = m_dataMaxY + padY;

    if (m_viewMinX < limitMinX) {
        m_viewMaxX += limitMinX - m_viewMinX;
        m_viewMinX = limitMinX;
    }
    if (m_viewMaxX > limitMaxX) {
        m_viewMinX -= m_viewMaxX - limitMaxX;
        m_viewMaxX = limitMaxX;
    }
    if (m_viewMinY < limitMinY) {
        m_viewMaxY += limitMinY - m_viewMinY;
        m_viewMinY = limitMinY;
    }
    if (m_viewMaxY > limitMaxY) {
        m_viewMinY -= m_viewMaxY - limitMaxY;
        m_viewMaxY = limitMaxY;
    }
}
