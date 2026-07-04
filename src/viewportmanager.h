#ifndef VIEWPORTMANAGER_H
#define VIEWPORTMANAGER_H

// Quản lý vùng nhìn (viewport): phạm vi min/max trục X/Y đang hiển thị trên màn hình.
// Tách khỏi DataManager để zoom/pan không đụng dữ liệu gốc — chỉ đổi cách shader ánh xạ tọa độ.
class ViewportManager
{
public:
    enum ZoomAxis {
        ZoomBoth = 0, // phóng to/thu nhỏ cả hai trục
        ZoomX    = 1, // chỉ trục X (ngang)
        ZoomY    = 2  // chỉ trục Y (dọc)
    };

    ViewportManager();

    void resetToDataBounds(float minX, float maxX, float minY, float maxY);
    void setViewBoundsX(float minX, float maxX);

    // factor > 1: zoom in (thu hẹp viewport), factor < 1: zoom out
    // anchorX/Y: điểm neo trong không gian dữ liệu (thường là vị trí con trỏ)
    void zoom(float factor, float anchorX, float anchorY, ZoomAxis axis);

    // Pan theo delta pixel trên widget; chuyển pixel → delta dữ liệu rồi dịch viewport
    void panPixels(float dx, float dy, float widgetWidth, float widgetHeight);

    float viewMinX() const { return m_viewMinX; }
    float viewMaxX() const { return m_viewMaxX; }
    float viewMinY() const { return m_viewMinY; }
    float viewMaxY() const { return m_viewMaxY; }

    // Chuyển tọa độ pixel (gốc trên-trái) sang tọa độ dữ liệu — dùng làm điểm neo khi zoom
    void pixelToData(float px, float py, float widgetWidth, float widgetHeight,
                     float &outX, float &outY) const;

private:
    float m_dataMinX = 0.0f;
    float m_dataMaxX = 1.0f;
    float m_dataMinY = 0.0f;
    float m_dataMaxY = 1.0f;

    float m_viewMinX = 0.0f;
    float m_viewMaxX = 1.0f;
    float m_viewMinY = 0.0f;
    float m_viewMaxY = 1.0f;

    void clampViewport();
    static void zoomOneAxis(float &viewMin, float &viewMax, float anchor, float factor, bool enabled);
};

#endif // VIEWPORTMANAGER_H
