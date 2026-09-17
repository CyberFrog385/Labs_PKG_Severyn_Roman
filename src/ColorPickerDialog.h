#ifndef COLORPICKERDIALOG_H
#define COLORPICKERDIALOG_H

#include <QDialog>
#include <QColor>
#include "ColorModel.h"

class QLabel;
class ColorWheel;
class GradientSlider;

class ColorPickerDialog : public QDialog {
public:
    explicit ColorPickerDialog(const QColor &initial = Qt::white,
                               QWidget *parent = nullptr);

    RGBColor getRgb() const { return currentRgb; }

private slots:
    void onWheelChanged(double h, double s);
    void onHueChanged(int h);
    void onSatChanged(int s);
    void onValueChanged(int v);

private:
    RGBColor currentRgb{1.0, 1.0, 1.0};
    bool isUpdating = false;
    double hue = 0.0;
    double sat = 0.0;
    double value = 1.0;

    ColorWheel *wheel = nullptr;
    GradientSlider *sliderH = nullptr;
    GradientSlider *sliderS = nullptr;
    GradientSlider *sliderV = nullptr;
    QLabel *preview = nullptr;

    void updateAll();
    void updateGradients();
    void pickColor(double h, double s, double v);
};

#endif