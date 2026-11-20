#include "imagerenderer.h"

#include <QColor>
#include <QTransform>

namespace {
int clampChannel(int value)
{
    return qBound(0, value, 255);
}
}

QImage ImageRenderer::render(const QImage &source, const EditSettings &settings)
{
    if (source.isNull()) {
        return {};
    }

    QImage result = source;
    if (settings.crop.isValid()) {
        result = applyCrop(result, settings.crop);
    }

    const int normalizedRotation = ((settings.rotation % 360) + 360) % 360;
    if (normalizedRotation != 0) {
        QTransform transform;
        transform.rotate(normalizedRotation);
        result = result.transformed(transform, Qt::SmoothTransformation);
    }
    if (settings.flipHorizontal || settings.flipVertical) {
        result = result.mirrored(settings.flipHorizontal, settings.flipVertical);
    }
    if (settings.brightness != 0) {
        result = applyBrightness(result, settings.brightness);
    }
    if (settings.grayscale) {
        QImage gray = result.convertToFormat(QImage::Format_ARGB32);
        for (int y = 0; y < gray.height(); ++y) {
            for (int x = 0; x < gray.width(); ++x) {
                const QColor color = gray.pixelColor(x, y);
                const int value = qGray(color.rgb());
                gray.setPixelColor(x, y, QColor(value, value, value, color.alpha()));
            }
        }
        result = gray;
    }
    return result;
}

QImage ImageRenderer::applyCrop(const QImage &source, const QRect &crop)
{
    if (source.isNull() || !crop.isValid()) {
        return source;
    }
    const QRect clipped = crop.intersected(source.rect());
    return clipped.isValid() ? source.copy(clipped) : source;
}

QImage ImageRenderer::applyBrightness(const QImage &source, int brightness)
{
    if (source.isNull() || brightness == 0) {
        return source;
    }

    QImage result = source.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < result.height(); ++y) {
        for (int x = 0; x < result.width(); ++x) {
            const QColor color = result.pixelColor(x, y);
            result.setPixelColor(x, y, QColor(clampChannel(color.red() + brightness),
                                              clampChannel(color.green() + brightness),
                                              clampChannel(color.blue() + brightness),
                                              color.alpha()));
        }
    }
    return result;
}
