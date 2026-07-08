#pragma once

#include <QObject>
#include <QString>
#include <QUrl>

class MediaCacheManager final : public QObject
{
    Q_OBJECT

public:
    explicit MediaCacheManager(QObject* parent = nullptr);

    QString rootPath() const;
    void setRootPath(const QString& path);

    int maxSizeMb() const;
    void setMaxSizeMb(int megabytes);

    qint64 usedBytes() const;
    QString usedBytesText() const;

    QString streamsDirectory() const;
    QString coversDirectory() const;

    QString streamFilePathFor(const QUrl& streamUrl) const;
    QString coverFilePathFor(const QString& songId) const;

    bool streamCacheExists(const QUrl& streamUrl) const;
    bool coverCacheExists(const QString& songId) const;

    void ensureDirectories() const;
    void enforceLimit();
    void clearAll();

    void loadSettings();
    void saveSettings() const;

    static QString defaultRootPath();

signals:
    void settingsChanged();
    void usageChanged();

private:
    qint64 directorySizeBytes(const QString& path) const;
    void removeOldestFiles(qint64 bytesToFree);

    QString m_rootPath;
    int m_maxSizeMb = 512;
};
