#include "photoviewerdialog.h"

#include "imagerenderer.h"

#include <QDir>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

PhotoViewerDialog::PhotoViewerDialog(const QList<PhotoRecord> &photos,
                                     const QString &projectDir,
                                     int startIndex,
                                     QWidget *parent)
    : QDialog(parent), _photos(photos), _projectDir(projectDir)
{
    setWindowTitle(tr("图片预览"));
    resize(900, 650);

    auto *layout = new QVBoxLayout(this);
    auto *toolbar = new QHBoxLayout;
    auto *previousButton = new QPushButton(tr("上一张"), this);
    auto *nextButton = new QPushButton(tr("下一张"), this);
    auto *fitButton = new QPushButton(tr("适应窗口"), this);
    auto *closeButton = new QPushButton(tr("关闭"), this);
    toolbar->addWidget(previousButton);
    toolbar->addWidget(nextButton);
    toolbar->addWidget(fitButton);
    toolbar->addStretch();
    toolbar->addWidget(closeButton);
    layout->addLayout(toolbar);

    _imageLabel = new QLabel(this);
    _imageLabel->setAlignment(Qt::AlignCenter);
    _imageLabel->setBackgroundRole(QPalette::Dark);
    _imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    _imageLabel->setScaledContents(false);

    _scrollArea = new QScrollArea(this);
    _scrollArea->setWidget(_imageLabel);
    _scrollArea->setWidgetResizable(true);
    layout->addWidget(_scrollArea, 1);

    connect(previousButton, &QPushButton::clicked, this, &PhotoViewerDialog::showPrevious);
    connect(nextButton, &QPushButton::clicked, this, &PhotoViewerDialog::showNext);
    connect(fitButton, &QPushButton::clicked, this, &PhotoViewerDialog::toggleFitMode);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    setPhotoIndex(startIndex);
}

void PhotoViewerDialog::setPhotoIndex(int index)
{
    if (_photos.isEmpty()) {
        _index = 0;
        _imageLabel->clear();
        return;
    }
    _index = (index % _photos.size() + _photos.size()) % _photos.size();
    updateImage();
}

void PhotoViewerDialog::showPrevious()
{
    setPhotoIndex(_index - 1);
}

void PhotoViewerDialog::showNext()
{
    setPhotoIndex(_index + 1);
}

void PhotoViewerDialog::toggleFitMode()
{
    _fitToWindow = !_fitToWindow;
    updateImage();
}

QImage PhotoViewerDialog::loadPhoto(const PhotoRecord &photo) const
{
    QImage image;
    const QString editedPath = QDir(_projectDir).filePath(QStringLiteral("edits/%1.png").arg(photo.id));
    if (!photo.edit.isDefault() && image.load(editedPath)) {
        return image;
    }
    image.load(QDir(_projectDir).filePath(photo.relativePath));
    return image.isNull() || photo.edit.isDefault() ? image : ImageRenderer::render(image, photo.edit);
}

void PhotoViewerDialog::updateImage()
{
    if (_photos.isEmpty()) {
        _imageLabel->clear();
        return;
    }
    const QImage image = loadPhoto(_photos.at(_index));
    if (image.isNull()) {
        _imageLabel->setText(tr("图片无法读取"));
        return;
    }
    QPixmap pixmap = QPixmap::fromImage(image);
    if (_fitToWindow) {
        pixmap = pixmap.scaled(_scrollArea->viewport()->size(),
                               Qt::KeepAspectRatio,
                               Qt::SmoothTransformation);
    }
    _imageLabel->setPixmap(pixmap);
    _imageLabel->adjustSize();
    setWindowTitle(tr("图片预览 - %1/%2").arg(_index + 1).arg(_photos.size()));
}
