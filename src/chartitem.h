#ifndef CHARTITEM_H
#define CHARTITEM_H
#include"strategies/linechartstrategy.h"
#include"strategies/barchartstrategy.h"
#include"strategies/piechartstrategy.h"
#include"src/datamanager.h"
#include <QQuickFramebufferObject>
#include<QOpenGLFunctions>
#include<QColor>


// lớp ChartItem chạy trên luồng giao diện (GUI), lắng nghe chuột, bàn phím, kích thước cửa sổ
class ChartItem : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_PROPERTY(int chartType READ chartType WRITE setChartType NOTIFY chartTypeChanged)
    Q_PROPERTY(QColor chartColor READ chartColor WRITE setChartColor NOTIFY chartColorChanged)

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

    //Mở cổng API cho QML gọi xuống
    Q_INVOKABLE bool loadDataFromFile(const QString &filePath);
    Q_INVOKABLE void clearChart();
signals:
    void chartTypeChanged(); // tín hiệu báo cho QML khi giá trị chartType thay đổi
    void chartColorChanged(); // tín hiệu phát ra khi màu thay đổi
private:
    int m_chartType=0; // line:0, bar:1
    QColor m_chartColor;// biến lưu màu ở C++

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

};

#endif // CHARTITEM_H
