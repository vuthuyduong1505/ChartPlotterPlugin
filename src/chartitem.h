#ifndef CHARTITEM_H
#define CHARTITEM_H

#include "src/datamanager.h"
#include "src/viewportmanager.h"
#include "src/onlinestream.h"
#include <QQuickFramebufferObject>
#include <QColor>
#include <QPointF>
#include <QUrl>
#include <QVariantMap>


// lớp ChartItem chạy trên luồng giao diện (GUI), lắng nghe chuột, bàn phím, kích thước cửa sổ
class ChartItem : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_PROPERTY(int chartType READ chartType WRITE setChartType NOTIFY chartTypeChanged)
    Q_PROPERTY(QColor chartColor READ chartColor WRITE setChartColor NOTIFY chartColorChanged)
    // 0: offline (file), 1: online (real-time stream)
    Q_PROPERTY(int dataMode READ dataMode WRITE setDataMode NOTIFY dataModeChanged)
    Q_PROPERTY(bool isAutoPanEnabled READ isAutoPanEnabled WRITE setIsAutoPanEnabled NOTIFY isAutoPanEnabledChanged)
    Q_PROPERTY(int lineStyle READ lineStyle WRITE setLineStyle NOTIFY lineStyleChanged)
    Q_PROPERTY(float zoomX READ zoomX WRITE setZoomX NOTIFY zoomXChanged)
    Q_PROPERTY(float zoomY READ zoomY WRITE setZoomY NOTIFY zoomYChanged)
    Q_PROPERTY(float panX READ panX WRITE setPanX NOTIFY panXChanged)
    Q_PROPERTY(float panY READ panY WRITE setPanY NOTIFY panYChanged)
    Q_PROPERTY(float dataMinX READ dataMinX NOTIFY dataBoundsChanged)
    Q_PROPERTY(float dataMaxX READ dataMaxX NOTIFY dataBoundsChanged)
    Q_PROPERTY(float dataMinY READ dataMinY NOTIFY dataBoundsChanged)
    Q_PROPERTY(float dataMaxY READ dataMaxY NOTIFY dataBoundsChanged)
    Q_PROPERTY(QVariantList xTicks READ xTicks NOTIFY xTicksChanged)
    Q_PROPERTY(QVariantList yTicks READ yTicks NOTIFY yTicksChanged)
    Q_PROPERTY(float viewMinX READ viewMinX NOTIFY xTicksChanged)
    Q_PROPERTY(float viewMaxX READ viewMaxX NOTIFY xTicksChanged)
    Q_PROPERTY(float viewMinY READ viewMinY NOTIFY yTicksChanged)
    Q_PROPERTY(float viewMaxY READ viewMaxY NOTIFY yTicksChanged)
    Q_PROPERTY(int pieBinMode READ pieBinMode WRITE setPieBinMode NOTIFY pieBinModeChanged)

public:
    ChartItem();

    //các hàm để đọc ghi biến chartType
    int chartType() const{
        return m_chartType;
    }
    void setChartType(int type); // hàm chạy mỗi khi QML chọn biểu đồ khác, ghi lại yêu cầu vào biến m_chartType
    Renderer *createRenderer() const override;

    // Các hàm đọc ghi biến chartColor
    QColor chartColor() const {return m_chartColor;}
    void setChartColor(const QColor &color);

    int dataMode() const { return m_dataMode; }
    void setDataMode(int mode);

    bool isAutoPanEnabled() const { return m_isAutoPanEnabled; }
    void setIsAutoPanEnabled(bool enabled);

    int lineStyle() const { return m_lineStyle; }
    void setLineStyle(int style);

    int pieBinMode() const { return m_pieBinMode; }
    void setPieBinMode(int mode);

    float zoomX() const;
    void setZoomX(float val);
    float zoomY() const;
    void setZoomY(float val);
    float panX() const;
    void setPanX(float val);
    float panY() const;
    void setPanY(float val);
    float dataMinX() const;
    float dataMaxX() const;
    float dataMinY() const;
    float dataMaxY() const;
    float viewMinX() const { return m_viewport.viewMinX(); }
    float viewMaxX() const { return m_viewport.viewMaxX(); }
    float viewMinY() const { return m_viewport.viewMinY(); }
    float viewMaxY() const { return m_viewport.viewMaxY(); }

    //Mở cổng API cho QML gọi xuống
    Q_INVOKABLE bool loadDataFromFile(const QUrl &fileUrl);
    Q_INVOKABLE void clearChart();
    Q_INVOKABLE void resetZoom();
    Q_INVOKABLE void pauseStream();
    Q_INVOKABLE void resumeStream();
    Q_INVOKABLE QVariantMap getNearestDataPoint(float mouseX, float mouseY, float screenWidth, float screenHeight);
    Q_INVOKABLE void createStressTestData();
    
    QVariantList xTicks() const;
    QVariantList yTicks() const;

protected:
    bool event(QEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void chartTypeChanged(); // tín hiệu báo cho QML khi giá trị chartType thay đổi
    void chartColorChanged(); // tín hiệu phát ra khi màu thay đổi
    void dataModeChanged();
    void isAutoPanEnabledChanged();
    void lineStyleChanged();
    void pieBinModeChanged();
    void zoomXChanged();
    void zoomYChanged();
    void panXChanged();
    void panYChanged();
    void dataBoundsChanged();
    void xTicksChanged();
    void yTicksChanged();

private:
    float calculateGridStep(float range) const;
    QVariantList calculateTicks(float minVal, float maxVal);
    void resetViewportFromData();
    ViewportManager::ZoomAxis zoomAxisFromModifiers(Qt::KeyboardModifiers mods) const;
    float zoomFactorFromWheel(const QWheelEvent *event) const;
    bool supportsViewportInteraction() const { return m_chartType != 2; } // Pie không dùng zoom/pan

    // Nhóm xử lý tương tác và dữ liệu (private helpers)
    void processPan(const QPointF& delta);
    void processZoom(float factor, const QPointF& center);
    void updateAutoPanLogic();

    int m_chartType=0; // line:0, bar:1, pie:2
    QColor m_chartColor;// biến lưu màu ở C++
    int m_dataMode = 0; // 0: offline, 1: online
    bool m_dataChanged = false; // đánh dấu dữ liệu thay đổi từ GUI thread
    bool m_viewChanged = false; // đánh dấu viewport thay đổi (zoom/pan)

    ViewportManager m_viewport;
    bool m_panning = false;
    QPointF m_lastPanPos;
    bool m_isAutoPanEnabled = true;
    int m_lineStyle = 0; // 0: solid, 1: dashed, 2: dotted
    int m_pieBinMode = 0; // 0: Theo X, 1: Theo 5 mức Y
    int m_hoveredSlice = -1; // Lát đang được di chuột vào (Exploded view)

    friend class ChartRenderer; // Cho phép Renderer truy cập m_dataChanged, m_viewport
};

#endif // CHARTITEM_H
