#include "albumproject.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>

#include <algorithm>

namespace {
constexpr int kProjectVersion = 1;
}

AlbumProject AlbumProject::create(const QString &parentPath,
                                  const QString &name,
                                  QString *error)
{
    AlbumProject project;
    const QString cleanParent = QDir(parentPath).absolutePath();
    const QString cleanName = name.trimmed();
    if (cleanName.isEmpty() || QFileInfo(cleanName).fileName() != cleanName
        || cleanName == "." || cleanName == "..") {
        project.setError(error, QStringLiteral("项目名称无效。"));
        return project;
    }

    QDir parent(cleanParent);
    if (!parent.exists()) {
        project.setError(error, QStringLiteral("项目父目录不存在。"));
        return project;
    }

    const QString projectDir = parent.absoluteFilePath(cleanName);
    if (QDir(projectDir).exists()) {
        project.setError(error, QStringLiteral("项目目录已经存在。"));
        return project;
    }

    QDir directory;
    if (!directory.mkpath(projectDir)
        || !QDir(projectDir).mkpath("photos")
        || !QDir(projectDir).mkpath("edits")
        || !QDir(projectDir).mkpath("music")) {
        project.setError(error, QStringLiteral("无法创建项目目录结构。"));
        return project;
    }

    project._projectDir = QDir::cleanPath(projectDir);
    project._name = cleanName;
    project._valid = true;
    project._dirty = true;
    if (!project.save(error)) {
        project = AlbumProject();
    }
    return project;
}

bool AlbumProject::open(const QString &projectPath, QString *error)
{
    const QFileInfo inputInfo(projectPath);
    QString directoryPath;
    if (inputInfo.isFile() && inputInfo.fileName().compare("album.json", Qt::CaseInsensitive) == 0) {
        directoryPath = inputInfo.absolutePath();
    } else if (inputInfo.isDir()) {
        directoryPath = inputInfo.absoluteFilePath();
    } else {
        setError(error, QStringLiteral("项目路径不存在，或不是 album.json。"));
        return false;
    }

    QFile file(QDir(directoryPath).filePath("album.json"));
    if (!file.open(QIODevice::ReadOnly)) {
        setError(error, QStringLiteral("无法读取项目文件：%1").arg(file.errorString()));
        return false;
    }
    const QByteArray contents = file.readAll();
    if (!loadJson(contents, error)) {
        return false;
    }
    _projectDir = QDir::cleanPath(directoryPath);
    if (!QDir(_projectDir).mkpath("photos")
        || !QDir(_projectDir).mkpath("edits")
        || !QDir(_projectDir).mkpath("music")) {
        setError(error, QStringLiteral("无法访问项目目录结构。"));
        return false;
    }
    _valid = true;
    _dirty = false;
    return true;
}

bool AlbumProject::save(QString *error) const
{
    if (!_valid && _projectDir.isEmpty()) {
        setError(error, QStringLiteral("项目尚未初始化。"));
        return false;
    }

    QSaveFile file(QDir(_projectDir).filePath("album.json"));
    if (!file.open(QIODevice::WriteOnly)) {
        setError(error, QStringLiteral("无法写入项目文件：%1").arg(file.errorString()));
        return false;
    }
    const QByteArray data = QJsonDocument(toJson()).toJson(QJsonDocument::Indented);
    if (file.write(data) != data.size() || !file.commit()) {
        setError(error, QStringLiteral("保存项目文件失败：%1").arg(file.errorString()));
        return false;
    }
    const_cast<AlbumProject *>(this)->_dirty = false;
    return true;
}

bool AlbumProject::isValid() const
{
    return _valid;
}

bool AlbumProject::isDirty() const
{
    return _dirty;
}

QString AlbumProject::projectDir() const
{
    return _projectDir;
}

QString AlbumProject::name() const
{
    return _name;
}

QList<PhotoRecord> AlbumProject::photos() const
{
    return _photos;
}

QList<PhotoRecord> AlbumProject::filteredPhotos() const
{
    QList<PhotoRecord> result;
    for (const PhotoRecord &photo : _photos) {
        if (_activeTag == QStringLiteral("全部") || photo.tags.contains(_activeTag)) {
            result.append(photo);
        }
    }

    if (_sortMode == QStringLiteral("name")) {
        std::sort(result.begin(), result.end(), [](const PhotoRecord &left, const PhotoRecord &right) {
            return QString::compare(left.name, right.name, Qt::CaseInsensitive) < 0;
        });
    } else {
        std::sort(result.begin(), result.end(), [](const PhotoRecord &left, const PhotoRecord &right) {
            return left.importedAt < right.importedAt;
        });
    }
    return result;
}

QStringList AlbumProject::tags() const
{
    return _tags;
}

QString AlbumProject::activeTag() const
{
    return _activeTag;
}

void AlbumProject::setActiveTag(const QString &tag)
{
    const QString next = tag.trimmed();
    const QString normalized = next.isEmpty() || next == QStringLiteral("全部") || !_tags.contains(next)
        ? QStringLiteral("全部")
        : next;
    if (_activeTag != normalized) {
        _activeTag = normalized;
        _dirty = true;
    }
}

void AlbumProject::setSortMode(const QString &mode)
{
    const QString normalized = mode == QStringLiteral("name") ? QStringLiteral("name")
                                                                : QStringLiteral("importedAt");
    if (_sortMode != normalized) {
        _sortMode = normalized;
        _dirty = true;
    }
}

QString AlbumProject::sortMode() const
{
    return _sortMode;
}

bool AlbumProject::addPhoto(PhotoRecord record)
{
    if (!_valid || record.id.trimmed().isEmpty() || record.relativePath.trimmed().isEmpty()
        || !QDir::isRelativePath(record.relativePath) || record.name.trimmed().isEmpty()) {
        return false;
    }
    for (const PhotoRecord &existing : _photos) {
        if (existing.id == record.id || existing.relativePath == record.relativePath) {
            return false;
        }
    }
    record.id = record.id.trimmed();
    record.relativePath = QDir::cleanPath(record.relativePath);
    record.name = record.name.trimmed();
    if (!record.importedAt.isValid()) {
        record.importedAt = QDateTime::currentDateTimeUtc();
    }
    record.tags = normalizeTags(record.tags);
    _photos.append(record);
    rebuildTags();
    _dirty = true;
    return true;
}

bool AlbumProject::updatePhoto(PhotoRecord record)
{
    for (PhotoRecord &existing : _photos) {
        if (existing.id == record.id) {
            if (record.relativePath.trimmed().isEmpty() || !QDir::isRelativePath(record.relativePath)) {
                return false;
            }
            record.tags = normalizeTags(record.tags);
            existing = record;
            rebuildTags();
            _dirty = true;
            return true;
        }
    }
    return false;
}

bool AlbumProject::removePhoto(const QString &id)
{
    for (int index = 0; index < _photos.size(); ++index) {
        if (_photos.at(index).id == id) {
            _photos.removeAt(index);
            rebuildTags();
            _dirty = true;
            return true;
        }
    }
    return false;
}

bool AlbumProject::renameTag(const QString &oldName, const QString &newName)
{
    const QString oldTag = normalizeTag(oldName);
    const QString newTag = normalizeTag(newName);
    if (oldTag.isEmpty() || newTag.isEmpty() || oldTag == newTag || !_tags.contains(oldTag)
        || _tags.contains(newTag)) {
        return false;
    }
    for (PhotoRecord &photo : _photos) {
        for (QString &tag : photo.tags) {
            if (tag == oldTag) {
                tag = newTag;
            }
        }
        photo.tags = normalizeTags(photo.tags);
    }
    if (_activeTag == oldTag) {
        _activeTag = newTag;
    }
    rebuildTags();
    _dirty = true;
    return true;
}

bool AlbumProject::removeTag(const QString &tag)
{
    const QString normalized = normalizeTag(tag);
    if (normalized.isEmpty() || !_tags.contains(normalized)) {
        return false;
    }
    for (PhotoRecord &photo : _photos) {
        photo.tags.removeAll(normalized);
    }
    rebuildTags();
    if (_activeTag == normalized) {
        _activeTag = QStringLiteral("全部");
    }
    _dirty = true;
    return true;
}

QString AlbumProject::editedFilePath(const PhotoRecord &record) const
{
    return QDir(_projectDir).filePath(QStringLiteral("edits/%1.png").arg(record.id));
}

QStringList AlbumProject::musicFiles() const
{
    return _musicFiles;
}

void AlbumProject::setMusicFiles(const QStringList &files)
{
    _musicFiles = files;
    _dirty = true;
}

bool AlbumProject::musicLoop() const
{
    return _musicLoop;
}

void AlbumProject::setMusicLoop(bool enabled)
{
    if (_musicLoop != enabled) {
        _musicLoop = enabled;
        _dirty = true;
    }
}

bool AlbumProject::loadJson(const QByteArray &contents, QString *error)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(contents, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        setError(error, QStringLiteral("项目文件格式错误：%1").arg(parseError.errorString()));
        return false;
    }

    const QJsonObject root = document.object();
    if (root.value("version").toInt(-1) != kProjectVersion) {
        setError(error, QStringLiteral("不支持的项目版本。"));
        return false;
    }
    const QString loadedName = root.value("name").toString().trimmed();
    if (loadedName.isEmpty()) {
        setError(error, QStringLiteral("项目名称为空。"));
        return false;
    }

    QList<PhotoRecord> loadedPhotos;
    const QJsonArray photoArray = root.value("photos").toArray();
    for (const QJsonValue &value : photoArray) {
        const PhotoRecord record = photoRecordFromJson(value.toObject());
        if (record.id.isEmpty() || record.relativePath.isEmpty() || !QDir::isRelativePath(record.relativePath)) {
            setError(error, QStringLiteral("项目中存在无效的图片路径。"));
            return false;
        }
        loadedPhotos.append(record);
    }

    _name = loadedName;
    _photos = loadedPhotos;
    _tags.clear();
    const QJsonArray tagArray = root.value("tags").toArray();
    for (const QJsonValue &value : tagArray) {
        const QString tag = normalizeTag(value.toString());
        if (!tag.isEmpty() && !_tags.contains(tag)) {
            _tags.append(tag);
        }
    }
    rebuildTags();
    _musicFiles.clear();
    const QJsonArray musicArray = root.value("music").toArray();
    for (const QJsonValue &value : musicArray) {
        const QString path = value.toString();
        if (!path.isEmpty() && QDir::isRelativePath(path)) {
            _musicFiles.append(QDir::cleanPath(path));
        }
    }
    _musicLoop = root.value("musicLoop").toBool(true);
    _activeTag = root.value("lastFilter").toString(QStringLiteral("全部"));
    if (_activeTag != QStringLiteral("全部") && !_tags.contains(_activeTag)) {
        _activeTag = QStringLiteral("全部");
    }
    setSortMode(root.value("sort").toString(QStringLiteral("importedAt")));
    _dirty = false;
    return true;
}

QJsonObject AlbumProject::toJson() const
{
    QJsonObject root;
    root.insert("version", kProjectVersion);
    root.insert("name", _name);

    QJsonArray photos;
    for (const PhotoRecord &photo : _photos) {
        photos.append(photoRecordToJson(photo));
    }
    root.insert("photos", photos);

    QJsonArray tags;
    for (const QString &tag : _tags) {
        tags.append(tag);
    }
    root.insert("tags", tags);

    QJsonArray music;
    for (const QString &file : _musicFiles) {
        music.append(file);
    }
    root.insert("music", music);
    root.insert("musicLoop", _musicLoop);
    root.insert("lastFilter", _activeTag);
    root.insert("sort", _sortMode);
    return root;
}

void AlbumProject::setError(QString *error, const QString &message) const
{
    if (error) {
        *error = message;
    }
}

QString AlbumProject::normalizeTag(const QString &tag)
{
    return tag.trimmed();
}

QStringList AlbumProject::normalizeTags(const QStringList &tags)
{
    QStringList result;
    for (const QString &tag : tags) {
        const QString normalized = normalizeTag(tag);
        if (!normalized.isEmpty() && !result.contains(normalized)) {
            result.append(normalized);
        }
    }
    return result;
}

void AlbumProject::rebuildTags()
{
    QStringList result = _tags;
    for (const PhotoRecord &photo : _photos) {
        for (const QString &tag : photo.tags) {
            if (!result.contains(tag)) {
                result.append(tag);
            }
        }
    }
    result.sort(Qt::CaseInsensitive);
    _tags = result;
    if (_activeTag != QStringLiteral("全部") && !_tags.contains(_activeTag)) {
        _activeTag = QStringLiteral("全部");
    }
}
