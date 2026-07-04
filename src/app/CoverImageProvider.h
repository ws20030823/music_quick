#pragma once

#include <QQuickImageProvider>

class AppController;

// CoverImageProvider — 将 AppController::currentCover() 暴露给 QML Image（image://cover/…）
class CoverImageProvider final : public QQuickImageProvider
{
public:
    explicit CoverImageProvider(AppController* controller);

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

private:
    AppController* m_controller = nullptr;
};
