// =============================================================================
// main.cpp — WingSound 程序入口
// =============================================================================
// 创建 QGuiApplication + QQmlApplicationEngine，将 AppController 注册为全局 "app"，
// 加载 QML 模块 MusicQuick 中的 Main.qml。
// =============================================================================

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QIcon>

#include "app/AppController.h"
#include "app/CoverImageProvider.h"

namespace {

void startupLog(const QString& message)
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::TempLocation)
        + QStringLiteral("/musicquick-startup.log");
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << message << '\n';
    }
}

} // namespace

int main(int argc, char* argv[])
{
    startupLog(QStringLiteral("main: enter"));
    QGuiApplication app(argc, argv);
    QQuickWindow::setDefaultAlphaBuffer(true);
    startupLog(QStringLiteral("main: QGuiApplication created"));
    QGuiApplication::setOrganizationName(QStringLiteral("WingSound"));
    QGuiApplication::setApplicationName(QStringLiteral("WingSound"));
    QGuiApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/logo.svg")));
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    QQmlApplicationEngine engine;
    auto* controller = new AppController(&engine);
    startupLog(QStringLiteral("main: AppController created"));

    engine.addImageProvider(QStringLiteral("cover"), new CoverImageProvider(controller));
    startupLog(QStringLiteral("main: image provider registered"));
    engine.rootContext()->setContextProperty(QStringLiteral("app"), controller);
    startupLog(QStringLiteral("main: context property set"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() {
            startupLog(QStringLiteral("main: objectCreationFailed"));
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.loadFromModule(QStringLiteral("MusicQuick"), QStringLiteral("Main"));
    startupLog(QStringLiteral("main: loadFromModule returned"));

    if (engine.rootObjects().isEmpty()) {
        startupLog(QStringLiteral("main: rootObjects empty"));
        return -1;
    }

    startupLog(QStringLiteral("main: entering event loop"));
    const int code = app.exec();
    startupLog(QStringLiteral("main: event loop exited ") + QString::number(code));
    return code;
}
