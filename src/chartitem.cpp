#include "chartitem.h"
#include "src/fileloader.h"
#include <QOpenGLFunctions>
#include <QOpenGLFramebufferObject>

ChartItem::ChartItem() {
    // Kết nối: khi DataManager báo có dữ liệu mới ChartItem tự động gọi update() để vẽ lại
    connect(DataManager::instance(), &DataManager::dataChanged,this,&ChartItem::update);
}

void ChartItem::setChartType(int type)
{
    if(m_chartType!=type){
        m_chartType=type;
        emit chartTypeChanged();
        update();
    }
}

bool ChartItem::loadDataFromFile(const QString &filePath)
{
    return FileLoader::loadDataset(filePath);
}

void ChartItem::clearChart()
{
    DataManager::instance()->clear();
}

void ChartRenderer::synchronize(QQuickFramebufferObject *item)
{
    ChartItem *view = static_cast<ChartItem*>(item);

    // Gửi loại biểu đồ (0, 1, 2...) từ Item xuống Renderer
    m_type = view->chartType();

}

// khi QML cần hiện một cái gì đó thì
//hàm này sẽ được gọi tự động để sinh ra hàm ChartRenderer
QQuickFramebufferObject::Renderer *ChartItem::createRenderer() const
{
    return new ChartRenderer();
}

//Hàm này được gọi đến và nhảy đến hàm render()
// đóng vai trò như hàm initializeGL()
ChartRenderer::ChartRenderer()
{
    strategy=nullptr;
}

ChartRenderer::~ChartRenderer()
{
    if (strategy) {
        delete strategy;
        strategy = nullptr;
    }
}

//đây là hàm chính để code OpenGL vẽ các biểu đồ
void ChartRenderer::render()
{
    QOpenGLFunctions *f =QOpenGLContext::currentContext()->functions();
    f->glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT);

    // nếu biểu đồ được chọn khác với biểu đồ hiện tại
    if (m_type != m_currentType || strategy == nullptr) {
        // Xóa chiến thuật cũ để tránh tốn RAM
        if (strategy)
        {
            delete strategy;
            strategy=nullptr;
        }

        // Factory Pattern đơn giản ở đây:
        if (m_type == 0) {
            strategy = new LineChartStrategy();
        } else if (m_type == 1) {
            strategy = new BarChartStrategy();  
        }
        else if(m_type==2)
        {
            strategy=new pieChartStrategy();

        }

        if(strategy!=nullptr) strategy->init(); // khởi tạo Shader/VBO cho biểu đồ mới
        m_currentType = m_type; // ghi nhớ loại biểu đồ hiện tại
    }

    // 2. THỰC HIỆN VẼ
    if (strategy) {
        strategy->draw(f, time);
    }
    time=time+0.05f;
    update();
}
// kết quả vẽ được dán vào bộ đệm này và hiện ra màn hình
QOpenGLFramebufferObject *ChartRenderer::createFramebufferObject(const QSize &size)
{
    return new QOpenGLFramebufferObject(size);
}


