#include <QFileInfo>
#include <QImage>
#include <QTemporaryDir>
#include <QtTest>

#include "albumproject.h"
#include "photoimporter.h"

namespace {
bool createTestImage(const QString &path, const QColor &color)
{
    QImage image(2, 2, QImage::Format_RGB32);
    image.fill(color);
    return image.save(path);
}
}

class PhotoImporterTest : public QObject
{
    Q_OBJECT

private slots:
    void copiesValidImagesWithoutOverwriting();
};

void PhotoImporterTest::copiesValidImagesWithoutOverwriting()
{
    QTemporaryDir source;
    QTemporaryDir projectDir;
    QVERIFY(source.isValid() && projectDir.isValid());
    QVERIFY(createTestImage(source.filePath("photo.png"), Qt::red));

    QString error;
    AlbumProject project = AlbumProject::create(projectDir.path(), "相册", &error);
    QVERIFY2(project.isValid(), qPrintable(error));

    const auto first = PhotoImporter::importFiles(project, {source.filePath("photo.png")});
    const auto second = PhotoImporter::importFiles(project, {source.filePath("photo.png")});
    QCOMPARE(first.imported.size(), 1);
    QCOMPARE(second.imported.size(), 1);
    QVERIFY(first.imported.first().relativePath != second.imported.first().relativePath);
    QVERIFY(QFileInfo(project.projectDir() + "/" + second.imported.first().relativePath).exists());
    QCOMPARE(project.photos().size(), 2);
}

QTEST_MAIN(PhotoImporterTest)
#include "test_photoimporter.moc"
