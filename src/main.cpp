#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <src/chartitem.h>
#include <QQuickWindow>

int main(int argc, char *argv[])
{
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    QGuiApplication app(argc, argv);

    qmlRegisterType<ChartItem>("MyChartLibrary", 1, 0, "MyChart");



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
