#include "slideshowdialog.h"

#include "imagerenderer.h"

#include <QDir>
#include <QHBoxLayout>
#include <QImage>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>
#include <QVBoxLayout>

SlideshowDialog::SlideshowDialog(const QList<PhotoRecord> &photos,
                                 const QString &projectDir,
                                 int intervalMs,
                                 bool loop,
                                 QWidget *parent)
    : QDialog(parent), _photos(photos), _projectDir(projectDir), _loop(loop)
{
    setWindowTitle(tr("相册轮播"));
    setWindowState(Qt::WindowFullScreen);
    setFocusPolicy(Qt::StrongFocus);

    auto *layout = new QVBoxLayout(this);
    _image = new QLabel(this);
    _image->setAlignment(Qt::AlignCenter);
    _image->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    layout->addWidget(_image, 1);
    _caption = new QLabel(this);
    _caption->setAlignment(Qt::AlignCenter);
    layout->addWidget(_caption);

    auto *controls = new QHBoxLayout;
    auto *previous = new QPushButton(tr("上一张"), this);
    auto *pause = new QPushButton(tr("暂停/继续"), this);
    auto *next = new QPushButton(tr("下一张"), this);
    auto *close = new QPushButton(tr("关闭"), this);
    controls->addStretch();
    controls->addWidget(previous);
    controls->addWidget(pause);
    controls->addWidget(next);
    controls->addWidget(close);
    controls->addStretch();
    layout->addLayout(controls);

    connect(previous, &QPushButton::clicked, this, &SlideshowDialog::previousPhoto);
    connect(pause, &QPushButton::clicked, this, &SlideshowDialog::togglePause);
    connect(next, &QPushButton::clicked, this, &SlideshowDialog::nextPhoto);
    connect(close, &QPushButton::clicked, this, &QDialog::accept);
    connect(&_timer, &QTimer::timeout, this, &SlideshowDialog::nextPhoto);

    _timer.setInterval(qMax(500, intervalMs));
    updateImage();
    if (_photos.size() > 1) {
        _timer.start();
    }
}

void SlideshowDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        accept();
        return;
    }
    if (event->key() == Qt::Key_Left) {
        previousPhoto();
        return;
    }
    if (event->key() == Qt::Key_Right || event->key() == Qt::Key_Space) {
        nextPhoto();
        return;
    }
    QDialog::keyPressEvent(event);
}

void SlideshowDialog::nextPhoto()
{
    if (_photos.isEmpty()) return;
    if (_index + 1 >= _photos.size()) {
        if (!_loop) {
            _timer.stop();
            return;
        }
        _index = 0;
    } else {
        ++_index;
    }
    updateImage();
}

void SlideshowDialog::previousPhoto()
{
    if (_photos.isEmpty()) return;
    if (_index == 0) {
        _index = _loop ? _photos.size() - 1 : 0;
    } else {
        --_index;
    }
    updateImage();
}

void SlideshowDialog::togglePause()
{
    if (_timer.isActive()) _timer.stop(); else if (_photos.size() > 1) _timer.start();
}

QImage SlideshowDialog::loadPhoto(const PhotoRecord &photo) const
{
    QImage image;
    const QString editedPath = QDir(_projectDir).filePath(QStringLiteral("edits/%1.png").arg(photo.id));
    if (!photo.edit.isDefault() && image.load(editedPath)) return image;
    image.load(QDir(_projectDir).filePath(photo.relativePath));
    return image.isNull() || photo.edit.isDefault() ? image : ImageRenderer::render(image, photo.edit);
}

void SlideshowDialog::updateImage()
{
    if (_photos.isEmpty()) {
        _image->setText(tr("没有可播放的图片"));
        _caption->clear();
        return;
    }
    const QImage image = loadPhoto(_photos.at(_index));
    if (image.isNull()) {
        _image->setText(tr("图片无法读取"));
    } else {
        _image->setPixmap(QPixmap::fromImage(image.scaled(size() - QSize(40, 160),
                                                          Qt::KeepAspectRatio,
                                                          Qt::SmoothTransformation)));
    }
    _caption->setText(tr("%1/%2  %3").arg(_index + 1).arg(_photos.size()).arg(_photos.at(_index).name));
}
