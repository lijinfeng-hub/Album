#include "photoimporter.h"

#include "albumproject.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QUuid>

#include <utility>

namespace {
QStringList supportedExtensions()
{
    QStringList extensions;
    for (const QByteArray &format : QImageReader::supportedImageFormats()) {
        extensions.append(QString::fromLatin1(format).toLower());
    }
    return extensions;
}

QString uniqueFileName(const QDir &directory, const QString &baseName, const QString &suffix)
{
    QString candidate = baseName + suffix;
    int index = 2;
    while (directory.exists(candidate)) {
        candidate = QStringLiteral("%1_%2%3").arg(baseName).arg(index++).arg(suffix);
    }
    return candidate;
}

void addFailure(PhotoImportSummary &summary, const QString &file, const QString &reason)
{
    summary.failed.append(file);
    summary.failureReasons.append(reason);
}
}

PhotoImportSummary PhotoImporter::importFiles(AlbumProject &project, const QStringList &files)
{
    PhotoImportSummary summary;
    if (!project.isValid()) {
        for (const QString &file : files) {
            addFailure(summary, file, QStringLiteral("当前项目无效。"));
        }
        return summary;
    }

    const QDir projectPhotos(project.projectDir() + QStringLiteral("/photos"));
    if (!projectPhotos.exists() && !QDir().mkpath(projectPhotos.absolutePath())) {
        for (const QString &file : files) {
            addFailure(summary, file, QStringLiteral("无法创建项目图片目录。"));
        }
        return summary;
    }

    for (const QString &sourcePath : files) {
        const QFileInfo sourceInfo(sourcePath);
        if (!sourceInfo.isFile() || !sourceInfo.isReadable()) {
            addFailure(summary, sourcePath, QStringLiteral("文件不存在或不可读取。"));
            continue;
        }

        QImageReader reader(sourceInfo.absoluteFilePath());
        if (!reader.canRead()) {
            addFailure(summary, sourcePath, reader.errorString().isEmpty()
                                            ? QStringLiteral("不是可读取的图片文件。")
                                            : reader.errorString());
            continue;
        }
        const QImage image = reader.read();
        if (image.isNull()) {
            addFailure(summary, sourcePath, reader.errorString().isEmpty()
                                            ? QStringLiteral("不是可读取的图片文件。")
                                            : reader.errorString());
            continue;
        }

        const QString baseName = sourceInfo.completeBaseName();
        const QString suffix = QStringLiteral(".") + sourceInfo.suffix().toLower();
        const QString targetName = uniqueFileName(projectPhotos, baseName, suffix);
        const QString targetPath = projectPhotos.filePath(targetName);
        if (!QFile::copy(sourceInfo.absoluteFilePath(), targetPath)) {
            addFailure(summary, sourcePath, QStringLiteral("复制图片失败。"));
            continue;
        }

        PhotoRecord record;
        record.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        record.relativePath = QDir(project.projectDir()).relativeFilePath(targetPath).replace('\\', '/');
        record.name = sourceInfo.fileName();
        record.importedAt = QDateTime::currentDateTimeUtc();
        if (!project.addPhoto(record)) {
            QFile::remove(targetPath);
            addFailure(summary, sourcePath, QStringLiteral("图片记录无法加入项目。"));
            continue;
        }
        summary.imported.append(record);
    }

    if (!summary.imported.isEmpty()) {
        QString error;
        if (!project.save(&error)) {
            for (const PhotoRecord &record : std::as_const(summary.imported)) {
                QFile::remove(QDir(project.projectDir()).filePath(record.relativePath));
                project.removePhoto(record.id);
            }
            const QString reason = error.isEmpty() ? QStringLiteral("保存项目失败。") : error;
            for (const QString &file : files) {
                if (!summary.failed.contains(file)) {
                    addFailure(summary, file, reason);
                }
            }
            summary.imported.clear();
        }
    }
    return summary;
}

QStringList PhotoImporter::imageFilesInDirectory(const QString &directoryPath)
{
    const QDir directory(directoryPath);
    if (!directory.exists()) {
        return {};
    }

    QStringList nameFilters;
    for (const QString &extension : supportedExtensions()) {
        nameFilters.append(QStringLiteral("*.%1").arg(extension));
    }
    return directory.entryList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);
}
