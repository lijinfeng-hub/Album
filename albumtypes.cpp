#include "albumtypes.h"

#include <QJsonArray>

bool EditSettings::isDefault() const
{
    return rotation == 0
        && !flipHorizontal
        && !flipVertical
        && brightness == 0
        && !grayscale
        && !crop.isValid();
}

QJsonObject editSettingsToJson(const EditSettings &settings)
{
    QJsonObject object;
    object.insert("rotation", settings.rotation);
    object.insert("flipHorizontal", settings.flipHorizontal);
    object.insert("flipVertical", settings.flipVertical);
    object.insert("brightness", settings.brightness);
    object.insert("grayscale", settings.grayscale);
    if (settings.crop.isValid()) {
        QJsonObject crop;
        crop.insert("x", settings.crop.x());
        crop.insert("y", settings.crop.y());
        crop.insert("width", settings.crop.width());
        crop.insert("height", settings.crop.height());
        object.insert("crop", crop);
    }
    return object;
}

EditSettings editSettingsFromJson(const QJsonObject &object)
{
    EditSettings settings;
    settings.rotation = object.value("rotation").toInt(0);
    settings.flipHorizontal = object.value("flipHorizontal").toBool(false);
    settings.flipVertical = object.value("flipVertical").toBool(false);
    settings.brightness = object.value("brightness").toInt(0);
    settings.grayscale = object.value("grayscale").toBool(false);

    const QJsonObject crop = object.value("crop").toObject();
    if (!crop.isEmpty()) {
        settings.crop = QRect(crop.value("x").toInt(),
                              crop.value("y").toInt(),
                              crop.value("width").toInt(),
                              crop.value("height").toInt());
    }
    return settings;
}

QJsonObject photoRecordToJson(const PhotoRecord &record)
{
    QJsonObject object;
    object.insert("id", record.id);
    object.insert("file", record.relativePath);
    object.insert("name", record.name);
    object.insert("importedAt", record.importedAt.toUTC().toString(Qt::ISODate));

    QJsonArray tags;
    for (const QString &tag : record.tags) {
        tags.append(tag);
    }
    object.insert("tags", tags);
    object.insert("edit", editSettingsToJson(record.edit));
    return object;
}

PhotoRecord photoRecordFromJson(const QJsonObject &object)
{
    PhotoRecord record;
    record.id = object.value("id").toString();
    record.relativePath = object.value("file").toString();
    record.name = object.value("name").toString();
    record.importedAt = QDateTime::fromString(object.value("importedAt").toString(), Qt::ISODate);
    if (!record.importedAt.isValid()) {
        record.importedAt = QDateTime::currentDateTimeUtc();
    }

    const QJsonArray tags = object.value("tags").toArray();
    for (const QJsonValue &tag : tags) {
        const QString value = tag.toString().trimmed();
        if (!value.isEmpty() && !record.tags.contains(value)) {
            record.tags.append(value);
        }
    }
    record.edit = editSettingsFromJson(object.value("edit").toObject());
    return record;
}
