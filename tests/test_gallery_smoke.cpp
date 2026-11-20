#include <QtTest>

#include "photogallerywidget.h"

class GallerySmokeTest : public QObject
{
    Q_OBJECT

private slots:
    void bindsPhotoRecords();
};

void GallerySmokeTest::bindsPhotoRecords()
{
    PhotoGalleryWidget gallery;
    QList<PhotoRecord> photos;
    PhotoRecord first;
    first.id = "one";
    first.name = "one.png";
    first.relativePath = "photos/one.png";
    PhotoRecord second = first;
    second.id = "two";
    second.name = "two.png";
    photos << first << second;
    gallery.setPhotos(photos, QString());
    QCOMPARE(gallery.count(), 2);
    QCOMPARE(gallery.item(0)->data(Qt::UserRole).toString(), QString("one"));
}

QTEST_MAIN(GallerySmokeTest)
#include "test_gallery_smoke.moc"
