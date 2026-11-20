#ifndef PHOTOIMPORTER_H
#define PHOTOIMPORTER_H

#include "albumtypes.h"

#include <QStringList>

class AlbumProject;

struct PhotoImportSummary
{
    QList<PhotoRecord> imported;
    QStringList failed;
    QStringList failureReasons;

    bool hasFailures() const { return !failed.isEmpty(); }
};

class PhotoImporter
{
public:
    static PhotoImportSummary importFiles(AlbumProject &project, const QStringList &files);
    static QStringList imageFilesInDirectory(const QString &directoryPath);
};

#endif // PHOTOIMPORTER_H
