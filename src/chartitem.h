#ifndef CHARTITEM_H
#define CHARTITEM_H

#include "src/datamanager.h"
#include "src/viewportmanager.h"
#include "src/onlinestream.h"
#include <QQuickFramebufferObject>
#include <QColor>
#include <QPointF>
#include <QUrl>


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

    //Mở cổng API cho QML gọi xuống
    Q_INVOKABLE bool loadDataFromFile(const QUrl &fileUrl);
    Q_INVOKABLE void clearChart();
    Q_INVOKABLE void resetZoom();
    Q_INVOKABLE void pauseStream();
    Q_INVOKABLE void resumeStream();

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

private:
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

    friend class ChartRenderer; // Cho phép Renderer truy cập m_dataChanged, m_viewport
};

#endif // CHARTITEM_H
