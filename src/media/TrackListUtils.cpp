#include "media/TrackListUtils.h"

#include "ui/TrackItemRoles.h"

#include <QBrush>
#include <QFileInfo>
#include <QFont>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPixmap>
#include <QSize>
#include <QVariant>

namespace TrackListUtils {

void applyMetadataToItem(QListWidgetItem* item,
                         const TrackMetadata& metadata,
                         const QString& filePath)
{
    const QFileInfo fileInfo(filePath);
    item->setData(TrackItemRoles::FilePath, filePath);
    // DisplayRole（text）给 Delegate 显示曲名；无标题时退回文件名
    item->setText(metadata.title.isEmpty() ? fileInfo.fileName() : metadata.title);
    item->setData(TrackItemRoles::Artist, metadata.artist);
    item->setData(TrackItemRoles::Album, metadata.album);
    item->setData(TrackItemRoles::Duration, metadata.durationText);
    item->setData(TrackItemRoles::IsPlaying, false);
    if(!metadata.cover.isNull())
    {
        item->setData(TrackItemRoles::CoverImage, QVariant::fromValue(metadata.cover));
    }
    item->setSizeHint(QSize(0, 56));
}

void updateHighlight(QListWidget* list, int playingRow)
{
    if(!list)
    {
        return;
    }

    for(int i = 0; i < list->count(); ++i)
    {
        QListWidgetItem* item = list->item(i);
        if(!item)
        {
            continue;
        }
        item->setData(TrackItemRoles::IsPlaying, i == playingRow);
        // 清除 Qt 默认选中样式，高亮完全由 Delegate 根据 IsPlaying 绘制
        item->setBackground(Qt::NoBrush);
        item->setForeground(QBrush());
        QFont font = item->font();
        font.setBold(false);
        item->setFont(font);
        item->setSizeHint(QSize(0, 56));
    }

    list->viewport()->update();
}

void updateLocalMusicStats(QLabel* statsLabel, const QListWidget* list)
{
    if(!statsLabel || !list)
    {
        return;
    }

    const int count = list->count();
    statsLabel->setText(
        count > 0
            ? QStringLiteral("共 %1 首歌曲").arg(count)
            : QStringLiteral("尚未导入歌曲"));
}

void updateCoverLabel(QLabel* label, const QImage& cover)
{
    if(!label)
    {
        return;
    }

    if(cover.isNull())
    {
        label->clear();
        return;
    }

    const QPixmap pixmap = QPixmap::fromImage(cover).scaled(
        label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    label->setText(QString());
    label->setPixmap(pixmap);
}

} // namespace TrackListUtils
