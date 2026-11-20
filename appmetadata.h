#ifndef APPMETADATA_H
#define APPMETADATA_H

#include <QString>

namespace AlbumMetadata
{
inline QString softwareName()
{
    return QStringLiteral("Album相册管理系统");
}

inline QString shortName()
{
    return QStringLiteral("Album");
}

inline QString version()
{
    return QStringLiteral("V1.0");
}

inline QString windowTitle()
{
    return QStringLiteral("%1 %2").arg(softwareName(), version());
}

inline QString copyrightText()
{
    return QStringLiteral("Album相册管理系统 %1\n\n基于 Qt 6 Widgets 和 Qt Multimedia 构建的桌面相册管理软件。\n支持图片导入、标签分类、图片编辑、轮播和背景音乐。")
        .arg(version());
}
}

#endif // APPMETADATA_H
