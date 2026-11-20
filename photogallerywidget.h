#ifndef PHOTOGALLERYWIDGET_H
#define PHOTOGALLERYWIDGET_H

#include "albumtypes.h"

#include <QListWidget>

class PhotoGalleryWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit PhotoGalleryWidget(QWidget *parent = nullptr);

    void setPhotos(const QList<PhotoRecord> &photos, const QString &projectDir);
    QList<PhotoRecord> photos() const;
    QStringList selectedPhotoIds() const;
    QString selectedPhotoId() const;

signals:
    void photoActivated(const QString &id);
    void selectionChanged(const QStringList &ids);
    void requestDelete(const QStringList &ids);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void emitSelectionChanged();
    void emitPhotoActivated(QListWidgetItem *item);

private:
    QPixmap placeholderPixmap() const;

    QList<PhotoRecord> _photos;
    QString _projectDir;
};

#endif // PHOTOGALLERYWIDGET_H
