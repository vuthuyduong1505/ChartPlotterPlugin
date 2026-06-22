#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <src/chartitem.h>
#include<QQuickWindow>
#include<QTimer>
int main(int argc, char *argv[])
{
     QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL); // bắt buộc máy tính phải dùng OpenGL
    QGuiApplication app(argc, argv);
     //đăng kí lớp C++ tên là ChartItem, trong file QML thì MyChart chính là lớp ChartItem
     qmlRegisterType<ChartItem>("MyChartLibrary", 1, 0, "MyChart");

     // Thiết lập máy bơm dữ liệu giả lập
     static float counter =0;
     QTimer dataPump;
     QObject::connect(&dataPump,&QTimer::timeout, [](){
         float y= (rand()%100); // giả lập nhiệt dộ ngẫu nhiên từ 0-100
             DataManager::instance()->addData(counter,y);
         counter+=1.0f;
         if(counter>100){
             counter=0; // rết khi đầy màn hình
             DataManager::instance()->clear();
         }
     });
         dataPump.start(100);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("QmlTest", "Main");

    return QGuiApplication::exec();
}
