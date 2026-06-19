#ifndef CHARTITEM_H
#define CHARTITEM_H
#include"linechartstrategy.h"
#include"barchartstrategy.h"
#include"piechartstrategy.h"
#include <QQuickFramebufferObject>
#include<QOpenGLFunctions>

// lớp ChartItem chạy trên luồng giao diện (GUI), lắng nghe chuột, bàn phím, kích thước cửa sổ
class ChartItem : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_PROPERTY(int chartType READ chartType WRITE setChartType NOTIFY chartTypeChanged)

public:
    ChartItem();

    //các hàm để đọc ghi biến chartType
    int chartType() const{
        return m_chartType;
    }
    void setChartType(int type); // hàm chạy mỗi khi QML chọn biểu đồ khác, ghi lại yêu cầu vào biến m_chartType
    Renderer *createRenderer() const override;

signals:
    void chartTypeChanged(); // tín hiệu báo cho QML khi giá trị chartType thay đổi
private:
    int m_chartType=0; // line:0, bar:1

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

};

#endif // CHARTITEM_H
