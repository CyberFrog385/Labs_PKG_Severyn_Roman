#include "ColorPickerDialog.h"
#include "ColorWheel.h"
#include "GradientSlider.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

ColorPickerDialog::ColorPickerDialog(const QColor &initial, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Выбор цвета");
    setMinimumWidth(300);

    const HSVColor initHsv = ColorModel::rgbToHsv({
        initial.redF(), initial.greenF(), initial.blueF()
    });
    hue = initHsv.h;
    sat = initHsv.s;
    value = initHsv.v;
    currentRgb = ColorModel::hsvToRgb({hue, sat, value});

    wheel = new ColorWheel(this);
    wheel->setValue(value);

    preview = new QLabel(this);
    preview->setFixedHeight(28);

    sliderH = new GradientSlider(Qt::Horizontal, this);
    sliderH->setRange(0, 359);
    sliderS = new GradientSlider(Qt::Horizontal, this);
    sliderS->setRange(0, 100);
    sliderV = new GradientSlider(Qt::Horizontal, this);
    sliderV->setRange(0, 100);

    auto *hsvLayout = new QFormLayout();
    hsvLayout->addRow("Тон (H):", sliderH);
    hsvLayout->addRow("Насыщенность (S):", sliderS);
    hsvLayout->addRow("Яркость (V):", sliderV);

    auto *presetLayout = new QHBoxLayout();
    const QColor presetColors[] = {
        QColor(255, 0, 0), QColor(255, 140, 0), QColor(255, 220, 0),
        QColor(50, 200, 50), QColor(0, 200, 255), QColor(30, 80, 255),
        QColor(160, 40, 220), QColor(0, 0, 0)
    };
    for (const QColor &c : presetColors) {
        auto *btn = new QPushButton(this);
        btn->setFixedSize(24, 24);
        btn->setStyleSheet(QString("background:%1; border:1px solid #888; border-radius:4px;")
                               .arg(c.name()));
        connect(btn, &QPushButton::clicked, this, [this, c] {
            const HSVColor hsv = ColorModel::rgbToHsv({c.redF(), c.greenF(), c.blueF()});
            pickColor(hsv.h, hsv.s, hsv.v);
        });
        presetLayout->addWidget(btn);
    }

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(wheel, 0, Qt::AlignHCenter);
    layout->addWidget(preview);
    layout->addLayout(hsvLayout);
    layout->addWidget(new QLabel("Готовые цвета", this));
    layout->addLayout(presetLayout);
    layout->addWidget(buttonBox);

    connect(wheel, &ColorWheel::hueSatChanged, this, &ColorPickerDialog::onWheelChanged);
    connect(sliderH, &QSlider::valueChanged, this, &ColorPickerDialog::onHueChanged);
    connect(sliderS, &QSlider::valueChanged, this, &ColorPickerDialog::onSatChanged);
    connect(sliderV, &QSlider::valueChanged, this, &ColorPickerDialog::onValueChanged);

    updateAll();
}

void ColorPickerDialog::pickColor(double h, double s, double v)
{
    hue = h;
    sat = s;
    value = v;
    currentRgb = ColorModel::hsvToRgb({hue, sat, value});
    updateAll();
}

void ColorPickerDialog::onWheelChanged(double h, double s)
{
    if (isUpdating)
        return;
    pickColor(h, s, value);
}

void ColorPickerDialog::onHueChanged(int h)
{
    if (isUpdating)
        return;
    pickColor(h, sat, value);
}

void ColorPickerDialog::onSatChanged(int s)
{
    if (isUpdating)
        return;
    pickColor(hue, s / 100.0, value);
}

void ColorPickerDialog::onValueChanged(int v)
{
    if (isUpdating)
        return;
    pickColor(hue, sat, v / 100.0);
    wheel->setValue(value);
}

void ColorPickerDialog::updateAll()
{
    isUpdating = true;

    wheel->setHueSat(hue, sat);
    sliderH->setValue(int(hue + 0.5));
    sliderS->setValue(int(sat * 100.0 + 0.5));
    sliderV->setValue(int(value * 100.0 + 0.5));

    preview->setStyleSheet(QString("background:%1; border:1px solid #999; border-radius:4px;")
                               .arg(QColor::fromRgbF(currentRgb.r, currentRgb.g, currentRgb.b).name()));

    updateGradients();

    isUpdating = false;
}

void ColorPickerDialog::updateGradients()
{
    QList<QColor> hueStops;
    for (int i = 0; i < 6; ++i) {
        const RGBColor c = ColorModel::hsvToRgb({double(i) * 60.0, 1.0, 1.0});
        hueStops.append(QColor::fromRgbF(c.r, c.g, c.b));
    }
    hueStops.append(hueStops.first());
    sliderH->setGradientStops(hueStops);

    const RGBColor sGray = ColorModel::hsvToRgb({hue, 0.0, value});
    const RGBColor sFull = ColorModel::hsvToRgb({hue, 1.0, value});
    sliderS->setGradientStops({
        QColor::fromRgbF(sGray.r, sGray.g, sGray.b),
        QColor::fromRgbF(sFull.r, sFull.g, sFull.b)
    });

    const RGBColor vColor = ColorModel::hsvToRgb({hue, sat, 1.0});
    sliderV->setGradientStops({
        QColor(0, 0, 0),
        QColor::fromRgbF(vColor.r, vColor.g, vColor.b)
    });
}