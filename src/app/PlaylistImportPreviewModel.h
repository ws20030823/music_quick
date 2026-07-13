#pragma once

#include "app/PlaylistStore.h"

#include <QAbstractListModel>
#include <QSet>
#include <QVector>

struct PlaylistImportPreviewRow {
    QString externalTitle;
    QString externalArtist;
    PlaylistTrackRef match;
    QString sourceLabel;
    bool imported = false;
    bool duplicate = false;

    bool hasMatch() const { return !match.songId.isEmpty(); }
};

class PlaylistImportPreviewModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        ExternalTitleRole = Qt::UserRole + 1,
        ExternalArtistRole,
        MatchedTitleRole,
        MatchedArtistRole,
        SourceLabelRole,
        StatusTextRole,
        StatusKindRole,
        HasMatchRole,
        ImportedRole,
        DuplicateRole,
    };

    explicit PlaylistImportPreviewModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void clear();
    void appendRow(const PlaylistImportPreviewRow& row);
    void markImported(const QSet<QString>& importedIds, const QSet<QString>& duplicateIds);

    const QVector<PlaylistImportPreviewRow>& rows() const;
    int matchedCount() const;
    int unmatchedCount() const;

private:
    QVector<PlaylistImportPreviewRow> m_rows;
};
