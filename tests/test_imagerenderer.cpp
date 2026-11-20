#include <QColor>
#include <QImage>
#include <QtTest>

#include "albumtypes.h"
#include "imagerenderer.h"

class ImageRendererTest : public QObject
{
    Q_OBJECT

private slots:
    void rotatesAndFlipsImage();
    void cropsAndAppliesBrightness();
};

void ImageRendererTest::rotatesAndFlipsImage()
{
    QImage source(2, 1, QImage::Format_RGB32);
    source.setPixelColor(0, 0, Qt::red);
    source.setPixelColor(1, 0, Qt::blue);

    EditSettings edit;
    edit.rotation = 90;
    const QImage rotated = ImageRenderer::render(source, edit);
    QCOMPARE(rotated.size(), QSize(1, 2));

    edit = {};
    edit.flipHorizontal = true;
    const QImage flipped = ImageRenderer::render(source, edit);
    QCOMPARE(flipped.pixelColor(0, 0), QColor(Qt::blue));
}

void ImageRendererTest::cropsAndAppliesBrightness()
{
    QImage source(4, 4, QImage::Format_RGB32);
    source.fill(QColor(100, 100, 100));
    EditSettings edit;
    edit.crop = QRect(1, 1, 2, 2);
    edit.brightness = 50;
    const QImage result = ImageRenderer::render(source, edit);
    QCOMPARE(result.size(), QSize(2, 2));
    QVERIFY(result.pixelColor(0, 0).red() > 100);
}

QTEST_MAIN(ImageRendererTest)
#include "test_imagerenderer.moc"
