#ifndef CHARTITEM_H
#define CHARTITEM_H
#include"strategies/linechartstrategy.h"
#include"strategies/barchartstrategy.h"
#include"strategies/piechartstrategy.h"
#include"src/datamanager.h"
#include"src/viewportmanager.h"
#include"src/onlinestream.h"
#include <QQuickFramebufferObject>
#include<QOpenGLFunctions>
#include<QColor>
#include<QPointF>
#include<QUrl>
#include<vector>


// lớp ChartItem chạy trên luồng giao diện (GUI), lắng nghe chuột, bàn phím, kích thước cửa sổ
class ChartItem : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_PROPERTY(int chartType READ chartType WRITE setChartType NOTIFY chartTypeChanged)
    Q_PROPERTY(QColor chartColor READ chartColor WRITE setChartColor NOTIFY chartColorChanged)
    // 0: offline (file), 1: online (real-time stream)
    Q_PROPERTY(int dataMode READ dataMode WRITE setDataMode NOTIFY dataModeChanged)

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

    //Mở cổng API cho QML gọi xuống
    Q_INVOKABLE bool loadDataFromFile(const QUrl &fileUrl);
    Q_INVOKABLE void clearChart();
    Q_INVOKABLE void resetZoom();

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

private:
    void resetViewportFromData();
    void applyZoomAt(float factor, const QPointF &pos, Qt::KeyboardModifiers mods);
    ViewportManager::ZoomAxis zoomAxisFromModifiers(Qt::KeyboardModifiers mods) const;
    float zoomFactorFromWheel(const QWheelEvent *event) const;
    bool supportsViewportInteraction() const { return m_chartType != 2; } // Pie không dùng zoom/pan

    int m_chartType=0; // line:0, bar:1, pie:2
    QColor m_chartColor;// biến lưu màu ở C++
    int m_dataMode = 0; // 0: offline, 1: online
    bool m_dataChanged = false; // đánh dấu dữ liệu thay đổi từ GUI thread
    bool m_viewChanged = false; // đánh dấu viewport thay đổi (zoom/pan)

    ViewportManager m_viewport;
    bool m_panning = false;
    QPointF m_lastPanPos;

    friend class ChartRenderer; // Cho phép Renderer truy cập m_dataChanged, m_viewport
};

// Luồng đồ họa
class ChartRenderer : public QQuickFramebufferObject::Renderer, protected QOpenGLFunctions
{
public:
    ChartRenderer();
    ~ChartRenderer();
    void render() override;
     void synchronize(QQuickFramebufferObject *item) override;
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;

private:
    ChartStrategy *strategy=nullptr; // biến chung cho mọi loại biểu đồ
    float time =0.0f;
    int m_type=0;
    int m_currentType=-1;
    QColor m_color;

    // Các biến đệm nội bộ trên Render Thread để đồng bộ hóa an toàn đa luồng
    std::vector<DataPoint> m_renderData;
    float m_dataMinX = 0.0f;
    float m_dataMaxX = 1.0f;
    float m_dataMinY = 0.0f;
    float m_dataMaxY = 1.0f;
    // Viewport: phạm vi đang hiển thị (truyền vào shader để zoom/pan)
    float m_viewMinX = 0.0f;
    float m_viewMaxX = 1.0f;
    float m_viewMinY = 0.0f;
    float m_viewMaxY = 1.0f;
    bool m_dataDirty = false; // đánh dấu dữ liệu VBO cần được cập nhật lại

};

#endif // CHARTITEM_H
