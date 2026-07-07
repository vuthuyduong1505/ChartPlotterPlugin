#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

int main(int argc, char *argv[])
{
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    QGuiApplication app(argc, argv);

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
