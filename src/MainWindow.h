#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QPushButton>
#include <QPalette>
#include <QEvent>
#include "ColorModel.h"
#include "CanvasModel.h"
#include "EditorController.h"
#include "CanvasView.h"

class QComboBox;
class QPushButton;
class QLabel;
class GradientSlider;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onRGBChanged();
    void onHSVChanged();
    void onXYZChanged();
    void onLabChanged();
    void openColorDialog();
    void saveImage();

private:
    ColorModel model;
    CanvasModel canvasModel;
    EditorController *editorController = nullptr;

    CanvasView *canvasView = nullptr;

    bool isUpdating = false;

    QSpinBox *spinR = nullptr;
    QSpinBox *spinG = nullptr;
    QSpinBox *spinB = nullptr;
    GradientSlider *sliderR = nullptr;
    GradientSlider *sliderG = nullptr;
    GradientSlider *sliderB = nullptr;

    QDoubleSpinBox *spinH = nullptr;
    QDoubleSpinBox *spinS = nullptr;
    QDoubleSpinBox *spinV = nullptr;

    QDoubleSpinBox *spinX = nullptr;
    QDoubleSpinBox *spinY = nullptr;
    QDoubleSpinBox *spinZ = nullptr;

    QDoubleSpinBox *spinL = nullptr;
    QDoubleSpinBox *spinA = nullptr;
    QDoubleSpinBox *spinB_ = nullptr;

    QWidget *colorPreview = nullptr;
    QPushButton *btnPalette = nullptr;
    QLabel *gamutWarning = nullptr;

    QComboBox *comboIlluminant = nullptr;
    QComboBox *comboGamut = nullptr;

    QPushButton *btnPen = nullptr;
    QPushButton *btnEraser = nullptr;
    QPushButton *btnSave = nullptr;

    void setupUI();
    void updateAllViews();
    void updateGradients();
    QDoubleSpinBox* createSpinBox(double min, double max, double step);

    bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif