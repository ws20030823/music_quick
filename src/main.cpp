// =============================================================================
// main.cpp — Music Quick 程序入口
// =============================================================================
// 创建 QGuiApplication + QQmlApplicationEngine，将 AppController 注册为全局 "app"，
// 加载 QML 模块 MusicQuick 中的 Main.qml。
// =============================================================================

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "app/AppController.h"

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    // 使用 Basic 风格，避免依赖系统原生控件主题
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    AppController controller;

    QQmlApplicationEngine engine;
    // QML 全局对象：所有 .qml 文件可通过 app.xxx 访问业务逻辑
    engine.rootContext()->setContextProperty(QStringLiteral("app"), &controller);
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule(QStringLiteral("MusicQuick"), QStringLiteral("Main"));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
