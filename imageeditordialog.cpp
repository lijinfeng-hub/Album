#include "imageeditordialog.h"

#include "imagerenderer.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>

ImageEditorDialog::ImageEditorDialog(const QImage &source,
                                     const EditSettings &settings,
                                     QWidget *parent)
    : QDialog(parent), _source(source), _settings(settings)
{
    setWindowTitle(tr("编辑图片"));
    resize(980, 700);

    auto *mainLayout = new QVBoxLayout(this);
    auto *contentLayout = new QHBoxLayout;
    auto *scrollArea = new QScrollArea(this);
    _preview = new QLabel(this);
    _preview->setAlignment(Qt::AlignCenter);
    _preview->setMinimumSize(420, 420);
    _preview->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    scrollArea->setWidget(_preview);
    scrollArea->setWidgetResizable(true);
    contentLayout->addWidget(scrollArea, 1);

    auto *controls = new QVBoxLayout;
    auto *rotateLayout = new QHBoxLayout;
    auto *leftButton = new QPushButton(tr("左转 90°"), this);
    auto *rightButton = new QPushButton(tr("右转 90°"), this);
    rotateLayout->addWidget(leftButton);
    rotateLayout->addWidget(rightButton);
    controls->addLayout(rotateLayout);

    _flipHorizontal = new QCheckBox(tr("水平翻转"), this);
    _flipVertical = new QCheckBox(tr("垂直翻转"), this);
    _grayscale = new QCheckBox(tr("灰度效果"), this);
    controls->addWidget(_flipHorizontal);
    controls->addWidget(_flipVertical);
    controls->addWidget(_grayscale);

    auto *brightnessLabel = new QLabel(tr("亮度"), this);
    _brightness = new QSlider(Qt::Horizontal, this);
    _brightness->setRange(-100, 100);
    _brightness->setValue(_settings.brightness);
    controls->addWidget(brightnessLabel);
    controls->addWidget(_brightness);

    auto *cropBox = new QGroupBox(tr("裁剪区域（原图坐标）"), this);
    auto *cropForm = new QFormLayout(cropBox);
    _cropEnabled = new QCheckBox(tr("启用裁剪"), cropBox);
    _cropX = new QSpinBox(cropBox);
    _cropY = new QSpinBox(cropBox);
    _cropWidth = new QSpinBox(cropBox);
    _cropHeight = new QSpinBox(cropBox);
    cropForm->addRow(_cropEnabled);
    cropForm->addRow(tr("X"), _cropX);
    cropForm->addRow(tr("Y"), _cropY);
    cropForm->addRow(tr("宽度"), _cropWidth);
    cropForm->addRow(tr("高度"), _cropHeight);
    controls->addWidget(cropBox);
    controls->addStretch();
    auto *resetButton = new QPushButton(tr("重置编辑"), this);
    controls->addWidget(resetButton);
    contentLayout->addLayout(controls);
    mainLayout->addLayout(contentLayout, 1);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttons);

    setSpinBoxRanges();
    _flipHorizontal->setChecked(_settings.flipHorizontal);
    _flipVertical->setChecked(_settings.flipVertical);
    _grayscale->setChecked(_settings.grayscale);
    _cropEnabled->setChecked(_settings.crop.isValid());
    if (_settings.crop.isValid()) {
        _cropX->setValue(_settings.crop.x());
        _cropY->setValue(_settings.crop.y());
        _cropWidth->setValue(_settings.crop.width());
        _cropHeight->setValue(_settings.crop.height());
    }

    connect(leftButton, &QPushButton::clicked, this, &ImageEditorDialog::rotateLeft);
    connect(rightButton, &QPushButton::clicked, this, &ImageEditorDialog::rotateRight);
    connect(_flipHorizontal, &QCheckBox::toggled, this, &ImageEditorDialog::updatePreview);
    connect(_flipVertical, &QCheckBox::toggled, this, &ImageEditorDialog::updatePreview);
    connect(_grayscale, &QCheckBox::toggled, this, &ImageEditorDialog::updatePreview);
    connect(_brightness, &QSlider::valueChanged, this, &ImageEditorDialog::updatePreview);
    connect(_cropEnabled, &QCheckBox::toggled, this, &ImageEditorDialog::updatePreview);
    connect(_cropX, qOverload<int>(&QSpinBox::valueChanged), this, &ImageEditorDialog::updatePreview);
    connect(_cropY, qOverload<int>(&QSpinBox::valueChanged), this, &ImageEditorDialog::updatePreview);
    connect(_cropWidth, qOverload<int>(&QSpinBox::valueChanged), this, &ImageEditorDialog::updatePreview);
    connect(_cropHeight, qOverload<int>(&QSpinBox::valueChanged), this, &ImageEditorDialog::updatePreview);
    connect(resetButton, &QPushButton::clicked, this, &ImageEditorDialog::resetSettings);
    connect(buttons, &QDialogButtonBox::accepted, this, &ImageEditorDialog::acceptEdit);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    updatePreview();
}

void ImageEditorDialog::rotateLeft()
{
    _settings.rotation = (_settings.rotation + 270) % 360;
    updatePreview();
}

void ImageEditorDialog::rotateRight()
{
    _settings.rotation = (_settings.rotation + 90) % 360;
    updatePreview();
}

void ImageEditorDialog::toggleHorizontalFlip()
{
    updatePreview();
}

void ImageEditorDialog::toggleVerticalFlip()
{
    updatePreview();
}

void ImageEditorDialog::updatePreview()
{
    const EditSettings settings = currentSettings();
    const QImage result = ImageRenderer::render(_source, settings);
    if (result.isNull()) {
        _preview->clear();
        return;
    }
    _preview->setPixmap(QPixmap::fromImage(result.scaled(680, 560,
                                                        Qt::KeepAspectRatio,
                                                        Qt::SmoothTransformation)));
}

void ImageEditorDialog::resetSettings()
{
    _settings = {};
    _brightness->setValue(0);
    _flipHorizontal->setChecked(false);
    _flipVertical->setChecked(false);
    _grayscale->setChecked(false);
    _cropEnabled->setChecked(false);
    _cropX->setValue(0);
    _cropY->setValue(0);
    _cropWidth->setValue(0);
    _cropHeight->setValue(0);
    updatePreview();
}

void ImageEditorDialog::acceptEdit()
{
    emit acceptedEdit(currentSettings(), ImageRenderer::render(_source, currentSettings()));
    accept();
}

EditSettings ImageEditorDialog::currentSettings() const
{
    EditSettings settings = _settings;
    settings.flipHorizontal = _flipHorizontal->isChecked();
    settings.flipVertical = _flipVertical->isChecked();
    settings.brightness = _brightness->value();
    settings.grayscale = _grayscale->isChecked();
    settings.crop = _cropEnabled->isChecked()
        ? QRect(_cropX->value(), _cropY->value(), _cropWidth->value(), _cropHeight->value())
        : QRect();
    return settings;
}

void ImageEditorDialog::setSpinBoxRanges()
{
    const int width = qMax(0, _source.width());
    const int height = qMax(0, _source.height());
    _cropX->setRange(0, width);
    _cropY->setRange(0, height);
    _cropWidth->setRange(0, width);
    _cropHeight->setRange(0, height);
    _cropWidth->setValue(width);
    _cropHeight->setValue(height);
}
