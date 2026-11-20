#ifndef PHOTOVIEWERDIALOG_H
#define PHOTOVIEWERDIALOG_H

#include "albumtypes.h"

#include <QDialog>

class QLabel;
class QScrollArea;

class PhotoViewerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PhotoViewerDialog(const QList<PhotoRecord> &photos,
                               const QString &projectDir,
                               int startIndex = 0,
                               QWidget *parent = nullptr);

public slots:
    void setPhotoIndex(int index);

private slots:
    void showPrevious();
    void showNext();
    void toggleFitMode();

private:
    QImage loadPhoto(const PhotoRecord &photo) const;
    void updateImage();

    QList<PhotoRecord> _photos;
    QString _projectDir;
    int _index = 0;
    bool _fitToWindow = true;
    QLabel *_imageLabel = nullptr;
    QScrollArea *_scrollArea = nullptr;
};

#endif // PHOTOVIEWERDIALOG_H
