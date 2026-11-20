#ifndef IMAGEEDITORDIALOG_H
#define IMAGEEDITORDIALOG_H

#include "albumtypes.h"

#include <QDialog>
#include <QImage>

class QLabel;
class QCheckBox;
class QSlider;
class QSpinBox;

class ImageEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImageEditorDialog(const QImage &source,
                                const EditSettings &settings = {},
                                QWidget *parent = nullptr);

signals:
    void acceptedEdit(const EditSettings &settings, const QImage &image);

private slots:
    void rotateLeft();
    void rotateRight();
    void toggleHorizontalFlip();
    void toggleVerticalFlip();
    void updatePreview();
    void resetSettings();
    void acceptEdit();

private:
    EditSettings currentSettings() const;
    void setSpinBoxRanges();

    QImage _source;
    EditSettings _settings;
    QLabel *_preview = nullptr;
    QSlider *_brightness = nullptr;
    QCheckBox *_grayscale = nullptr;
    QCheckBox *_cropEnabled = nullptr;
    QCheckBox *_flipHorizontal = nullptr;
    QCheckBox *_flipVertical = nullptr;
    QSpinBox *_cropX = nullptr;
    QSpinBox *_cropY = nullptr;
    QSpinBox *_cropWidth = nullptr;
    QSpinBox *_cropHeight = nullptr;
};

#endif // IMAGEEDITORDIALOG_H
