#ifndef ALBUMPROJECT_H
#define ALBUMPROJECT_H

#include "albumtypes.h"

#include <QList>
#include <QStringList>

class AlbumProject
{
public:
    static AlbumProject create(const QString &parentPath,
                               const QString &name,
                               QString *error = nullptr);

    bool open(const QString &projectPath, QString *error = nullptr);
    bool save(QString *error = nullptr) const;

    bool isValid() const;
    bool isDirty() const;
    QString projectDir() const;
    QString name() const;

    QList<PhotoRecord> photos() const;
    QList<PhotoRecord> filteredPhotos() const;
    QStringList tags() const;
    QString activeTag() const;
    void setActiveTag(const QString &tag);
    void setSortMode(const QString &mode);
    QString sortMode() const;

    bool addPhoto(PhotoRecord record);
    bool updatePhoto(PhotoRecord record);
    bool removePhoto(const QString &id);
    bool renameTag(const QString &oldName, const QString &newName);
    bool removeTag(const QString &tag);

    QString editedFilePath(const PhotoRecord &record) const;

    QStringList musicFiles() const;
    void setMusicFiles(const QStringList &files);
    bool musicLoop() const;
    void setMusicLoop(bool enabled);

private:
    bool loadJson(const QByteArray &contents, QString *error);
    QJsonObject toJson() const;
    void setError(QString *error, const QString &message) const;
    static QString normalizeTag(const QString &tag);
    static QStringList normalizeTags(const QStringList &tags);
    void rebuildTags();

    QString _projectDir;
    QString _name;
    QList<PhotoRecord> _photos;
    QStringList _tags;
    QStringList _musicFiles;
    QString _activeTag = QStringLiteral("全部");
    QString _sortMode = QStringLiteral("importedAt");
    bool _musicLoop = true;
    bool _valid = false;
    bool _dirty = false;
};

#endif // ALBUMPROJECT_H
