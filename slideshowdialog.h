#ifndef SLIDESHOWDIALOG_H
#define SLIDESHOWDIALOG_H

#include "albumtypes.h"

#include <QDialog>
#include <QTimer>

class QLabel;

class SlideshowDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SlideshowDialog(const QList<PhotoRecord> &photos,
                             const QString &projectDir,
                             int intervalMs = 3000,
                             bool loop = true,
                             QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void nextPhoto();
    void previousPhoto();
    void togglePause();

private:
    QImage loadPhoto(const PhotoRecord &photo) const;
    void updateImage();

    QList<PhotoRecord> _photos;
    QString _projectDir;
    QTimer _timer;
    QLabel *_image = nullptr;
    QLabel *_caption = nullptr;
    int _index = 0;
    bool _loop = true;
};

#endif // SLIDESHOWDIALOG_H
