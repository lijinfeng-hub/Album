#ifndef IMAGERENDERER_H
#define IMAGERENDERER_H

#include "albumtypes.h"

#include <QImage>

class ImageRenderer
{
public:
    static QImage render(const QImage &source, const EditSettings &settings);
    static QImage applyCrop(const QImage &source, const QRect &crop);
    static QImage applyBrightness(const QImage &source, int brightness);
};

#endif // IMAGERENDERER_H
