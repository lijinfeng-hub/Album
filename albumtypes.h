#ifndef ALBUMTYPES_H
#define ALBUMTYPES_H

#include <QDateTime>
#include <QJsonObject>
#include <QRect>
#include <QStringList>

struct EditSettings
{
    int rotation = 0;
    bool flipHorizontal = false;
    bool flipVertical = false;
    int brightness = 0;
    bool grayscale = false;
    QRect crop;

    bool isDefault() const;
};

struct PhotoRecord
{
    QString id;
    QString relativePath;
    QString name;
    QStringList tags;
    QDateTime importedAt;
    EditSettings edit;
};

QJsonObject editSettingsToJson(const EditSettings &settings);
EditSettings editSettingsFromJson(const QJsonObject &object);
QJsonObject photoRecordToJson(const PhotoRecord &record);
PhotoRecord photoRecordFromJson(const QJsonObject &object);

#endif // ALBUMTYPES_H
