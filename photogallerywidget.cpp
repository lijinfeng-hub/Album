#include "photogallerywidget.h"

#include "imagerenderer.h"

#include <QContextMenuEvent>
#include <QDir>
#include <QImage>
#include <QMenu>
#include <QPainter>

namespace {
constexpr int kThumbnailWidth = 170;
constexpr int kThumbnailHeight = 130;
}

PhotoGalleryWidget::PhotoGalleryWidget(QWidget *parent)
    : QListWidget(parent)
{
    setObjectName(QStringLiteral("photoGallery"));
    setViewMode(QListView::IconMode);
    setIconSize(QSize(kThumbnailWidth, kThumbnailHeight));
    setGridSize(QSize(kThumbnailWidth + 24, kThumbnailHeight + 52));
    setResizeMode(QListView::Adjust);
    setMovement(QListView::Static);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSpacing(8);
    setWordWrap(true);
    setUniformItemSizes(true);
    setContextMenuPolicy(Qt::DefaultContextMenu);

    connect(this, &QListWidget::itemSelectionChanged,
            this, &PhotoGalleryWidget::emitSelectionChanged);
    connect(this, &QListWidget::itemDoubleClicked,
            this, &PhotoGalleryWidget::emitPhotoActivated);
    connect(this, &QListWidget::itemActivated,
            this, &PhotoGalleryWidget::emitPhotoActivated);
}

void PhotoGalleryWidget::setPhotos(const QList<PhotoRecord> &photos, const QString &projectDir)
{
    _photos = photos;
    _projectDir = projectDir;
    clear();

    for (int index = 0; index < _photos.size(); ++index) {
        const PhotoRecord &photo = _photos.at(index);
        auto *item = new QListWidgetItem(photo.name, this);
        item->setData(Qt::UserRole, photo.id);
        item->setData(Qt::UserRole + 1, index);
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignTop);
        item->setToolTip(photo.relativePath);

        QImage image;
        if (!_projectDir.isEmpty()) {
            const QString editedPath = QDir(_projectDir).filePath(QStringLiteral("edits/%1.png").arg(photo.id));
            if (!photo.edit.isDefault() && image.load(editedPath)) {
                // Use the persisted edit result when it is available.
            } else {
                image.load(QDir(_projectDir).filePath(photo.relativePath));
                if (!image.isNull() && !photo.edit.isDefault()) {
                    image = ImageRenderer::render(image, photo.edit);
                }
            }
        }
        if (image.isNull()) {
            item->setIcon(QIcon(placeholderPixmap()));
        } else {
            item->setIcon(QIcon(QPixmap::fromImage(image.scaled(iconSize(),
                                                               Qt::KeepAspectRatio,
                                                               Qt::SmoothTransformation))));
        }
    }
    emitSelectionChanged();
}

QList<PhotoRecord> PhotoGalleryWidget::photos() const
{
    return _photos;
}

QStringList PhotoGalleryWidget::selectedPhotoIds() const
{
    QStringList ids;
    for (QListWidgetItem *item : selectedItems()) {
        ids.append(item->data(Qt::UserRole).toString());
    }
    return ids;
}

QString PhotoGalleryWidget::selectedPhotoId() const
{
    const QListWidgetItem *item = currentItem();
    return item ? item->data(Qt::UserRole).toString() : QString();
}

void PhotoGalleryWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    QAction *deleteAction = menu.addAction(tr("删除选中图片"));
    deleteAction->setEnabled(!selectedItems().isEmpty());
    if (menu.exec(event->globalPos()) == deleteAction && !selectedItems().isEmpty()) {
        emit requestDelete(selectedPhotoIds());
    }
}

void PhotoGalleryWidget::emitSelectionChanged()
{
    emit selectionChanged(selectedPhotoIds());
}

void PhotoGalleryWidget::emitPhotoActivated(QListWidgetItem *item)
{
    if (item) {
        emit photoActivated(item->data(Qt::UserRole).toString());
    }
}

QPixmap PhotoGalleryWidget::placeholderPixmap() const
{
    QPixmap pixmap(kThumbnailWidth, kThumbnailHeight);
    pixmap.fill(QColor(58, 59, 60));
    QPainter painter(&pixmap);
    painter.setPen(QColor(180, 180, 180));
    painter.drawText(pixmap.rect(), Qt::AlignCenter, tr("无法预览"));
    return pixmap;
}
