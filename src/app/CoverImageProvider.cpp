#include "app/CoverImageProvider.h"

#include "app/AppController.h"

CoverImageProvider::CoverImageProvider(AppController* controller)
    : QQuickImageProvider(QQuickImageProvider::Image)
    , m_controller(controller)
{
}

QImage CoverImageProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize)
{
    Q_UNUSED(id);
    Q_UNUSED(requestedSize);

    if (!m_controller || !m_controller->hasCover()) {
        if (size) {
            *size = QSize(0, 0);
        }
        return {};
    }

    const QImage image = m_controller->currentCover();
    if (size) {
        *size = image.size();
    }
    return image;
}
