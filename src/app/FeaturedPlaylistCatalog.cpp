#include "app/FeaturedPlaylistCatalog.h"

namespace {

constexpr auto kCoverPrefix = "qrc:/images/";

const FeaturedPlaylist kCatalog[] = {
    {
        QStringLiteral("xue-zhiqian"),
        QStringLiteral("深情薛之谦"),
        QStringLiteral("陪你去流浪的那些瞬间"),
        QStringLiteral("薛之谦"),
        QString::fromLatin1(kCoverPrefix) + QStringLiteral("xue-zhiqian.jpg"),
        QStringLiteral("gequbao"),
    },
    {
        QStringLiteral("jay-chou"),
        QStringLiteral("最全周杰伦精选"),
        QStringLiteral("地表最强，回忆杀集合"),
        QStringLiteral("周杰伦"),
        QString::fromLatin1(kCoverPrefix) + QStringLiteral("jay-chou.jpg"),
        QStringLiteral("gequbao"),
    },
    {
        QStringLiteral("eason"),
        QStringLiteral("单单 Eason"),
        QStringLiteral("治愈与孤独的经典情歌"),
        QStringLiteral("陈奕迅"),
        QString::fromLatin1(kCoverPrefix) + QStringLiteral("eason.jpg"),
        QStringLiteral("gequbao"),
    },
    {
        QStringLiteral("gem"),
        QStringLiteral("G.E.M. 热门合集"),
        QStringLiteral("铁肺天后的力量感盛宴"),
        QStringLiteral("邓紫棋"),
        QString::fromLatin1(kCoverPrefix) + QStringLiteral("gem.jpg"),
        QStringLiteral("gequbao"),
    },
    {
        QStringLiteral("zhao-lei"),
        QStringLiteral("赵雷精选"),
        QStringLiteral("成都路上的民谣时光"),
        QStringLiteral("赵雷"),
        QString::fromLatin1(kCoverPrefix) + QStringLiteral("zhao-lei.jpg"),
        QStringLiteral("gequbao"),
    },
    {
        QStringLiteral("wang-leehom"),
        QStringLiteral("王力宏精选"),
        QStringLiteral("华语流行经典之作"),
        QStringLiteral("王力宏"),
        QString::fromLatin1(kCoverPrefix) + QStringLiteral("wang-leehom.jpg"),
        QStringLiteral("gequbao"),
    },
    {
        QStringLiteral("jj-lin"),
        QStringLiteral("林俊杰热门"),
        QStringLiteral("JJ 式情歌全收录"),
        QStringLiteral("林俊杰"),
        QString::fromLatin1(kCoverPrefix) + QStringLiteral("jj-lin.jpg"),
        QStringLiteral("gequbao"),
    },
    {
        QStringLiteral("liang-bo"),
        QStringLiteral("梁博作品集"),
        QStringLiteral("独立摇滚的赤诚表达"),
        QStringLiteral("梁博"),
        QString::fromLatin1(kCoverPrefix) + QStringLiteral("liang-bo.jpg"),
        QStringLiteral("gequbao"),
    },
};

} // namespace

QList<FeaturedPlaylist> FeaturedPlaylistCatalog::all()
{
    return { std::begin(kCatalog), std::end(kCatalog) };
}

const FeaturedPlaylist* FeaturedPlaylistCatalog::find(const QString& id)
{
    for (const FeaturedPlaylist& item : kCatalog) {
        if (item.id == id) {
            return &item;
        }
    }
    return nullptr;
}
