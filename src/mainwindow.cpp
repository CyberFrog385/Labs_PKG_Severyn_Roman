#include "MainWindow.h"
#include "GradientSlider.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include "ColorPickerDialog.h"
#include <algorithm>
#include <cmath>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    canvasModel(QSize(800, 600))
{
    editorController = new EditorController(&canvasModel, this);

    setupUI();
    updateAllViews();
}

QDoubleSpinBox* MainWindow::createSpinBox(double min, double max, double step) {
    auto *sb = new QDoubleSpinBox(this);
    sb->setRange(min, max);
    sb->setSingleStep(step);
    sb->setDecimals(2);
    sb->setFixedWidth(90);
    return sb;
}

void MainWindow::setupUI() {
    auto *centralWidget = new QWidget(this);
    auto *mainLayout = new QHBoxLayout(centralWidget);

    auto *rgbGroup = new QGroupBox("RGB", this);
    auto *rgbLayout = new QFormLayout(rgbGroup);

    auto addRgbRow = [this, rgbLayout](const QString &label, GradientSlider *&slider, QSpinBox *&spin) {
        slider = new GradientSlider(Qt::Horizontal, this);
        slider->setRange(0, 255);
        spin = new QSpinBox(this);
        spin->setRange(0, 255);
        spin->setFixedWidth(90);
        auto *row = new QHBoxLayout();
        row->addWidget(slider, 1);
        row->addWidget(spin);
        rgbLayout->addRow(label, row);
    };
    addRgbRow("R:", sliderR, spinR);
    addRgbRow("G:", sliderG, spinG);
    addRgbRow("B:", sliderB, spinB);

    auto *hsvGroup = new QGroupBox("HSV", this);
    auto *hsvLayout = new QFormLayout(hsvGroup);
    spinH = createSpinBox(0.0, 360.0, 1.0);
    spinS = createSpinBox(0.0, 1.0, 0.01);
    spinV = createSpinBox(0.0, 1.0, 0.01);
    hsvLayout->addRow("H (Hue):", spinH);
    hsvLayout->addRow("S (Saturation):", spinS);
    hsvLayout->addRow("V (Value):", spinV);

    auto *xyzGroup = new QGroupBox("XYZ", this);
    auto *xyzLayout = new QFormLayout(xyzGroup);
    spinX = createSpinBox(0.0, 95.047, 0.5);
    spinY = createSpinBox(0.0, 100.000, 0.5);
    spinZ = createSpinBox(0.0, 108.883, 0.5);
    xyzLayout->addRow("X:", spinX);
    xyzLayout->addRow("Y:", spinY);
    xyzLayout->addRow("Z:", spinZ);

    auto *labGroup = new QGroupBox("LAB", this);
    auto *labLayout = new QFormLayout(labGroup);
    spinL = createSpinBox(0.0, 100.0, 0.5);
    spinA = createSpinBox(-128.0, 127.0, 0.5);
    spinB_ = createSpinBox(-128.0, 127.0, 0.5);
    labLayout->addRow("L:", spinL);
    labLayout->addRow("A:", spinA);
    labLayout->addRow("B:", spinB_);

    QFont groupFont = hsvGroup->font();
    groupFont.setBold(true);
    groupFont.setPointSizeF(groupFont.pointSizeF() * 1.2);
    rgbGroup->setFont(groupFont);
    hsvGroup->setFont(groupFont);
    xyzGroup->setFont(groupFont);
    labGroup->setFont(groupFont);

    comboIlluminant = new QComboBox(this);
    comboIlluminant->addItem("D65 (дневной свет)", static_cast<int>(Illuminant::D65));
    comboIlluminant->addItem("D50 (полиграфия)", static_cast<int>(Illuminant::D50));
    comboIlluminant->addItem("E (равноэнергетический)", static_cast<int>(Illuminant::E));

    comboGamut = new QComboBox(this);
    comboGamut->addItem("Обрезка (Clipping)", static_cast<int>(GamutStrategy::Clipping));
    comboGamut->addItem("Масштабирование (Scaling)", static_cast<int>(GamutStrategy::Scaling));

    colorPreview = new QWidget(this);
    colorPreview->setMinimumSize(100, 100);
    colorPreview->setAutoFillBackground(true);
    colorPreview->setCursor(Qt::PointingHandCursor);
    colorPreview->installEventFilter(this);

    btnPalette = new QPushButton("Выбрать из палитры", this);

    auto *colorPanelLayout = new QVBoxLayout();
    colorPanelLayout->addWidget(new QLabel("Источник света", this));
    colorPanelLayout->addWidget(comboIlluminant);
    colorPanelLayout->addWidget(new QLabel("Выход за гамму", this));
    colorPanelLayout->addWidget(comboGamut);

    gamutWarning = new QLabel("\u26A0 Выход за пределы RGB-гаммы", this);
    gamutWarning->setStyleSheet(
        "background: #fff3cd;"
        "border: 1px solid #ffd08a;"
        "color: #8a5a00;"
        "padding: 2px 8px;"
        "border-radius: 3px;");
    gamutWarning->setWordWrap(true);
    gamutWarning->setVisible(false);
    colorPanelLayout->addWidget(gamutWarning);
    colorPanelLayout->addWidget(rgbGroup);
    colorPanelLayout->addWidget(hsvGroup);
    colorPanelLayout->addWidget(xyzGroup);
    colorPanelLayout->addWidget(labGroup);
    colorPanelLayout->addWidget(colorPreview);
    colorPanelLayout->addWidget(btnPalette);

    auto *toolsGroup = new QGroupBox("Инструменты рисования", this);
    auto *toolsLayout = new QVBoxLayout(toolsGroup);

    btnSave = new QPushButton("Сохранить изменения", this);
    toolsLayout->addWidget(btnSave);

    btnPen = new QPushButton("Кисть", this);
    btnEraser = new QPushButton("Ластик", this);

    btnPen->setCheckable(true);
    btnEraser->setCheckable(true);
    btnPen->setChecked(true);

    auto *spinWidth = new QSpinBox(this);
    spinWidth->setRange(1, 50);
    spinWidth->setValue(5);

    auto *widthLayout = new QHBoxLayout();
    widthLayout->addWidget(new QLabel("Размер:", this));
    widthLayout->addWidget(spinWidth);

    toolsLayout->addWidget(btnPen);
    toolsLayout->addWidget(btnEraser);
    toolsLayout->addLayout(widthLayout);

    colorPanelLayout->addWidget(toolsGroup);
    colorPanelLayout->addStretch();

    canvasView = new CanvasView(&canvasModel, editorController, this);

    mainLayout->addLayout(colorPanelLayout, 0);
    mainLayout->addWidget(canvasView, 1);

    setCentralWidget(centralWidget);
    setWindowTitle("ItteC");

    connect(spinR, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onRGBChanged);
    connect(spinG, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onRGBChanged);
    connect(spinB, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onRGBChanged);

    connect(sliderR, &QSlider::valueChanged, spinR, &QSpinBox::setValue);
    connect(sliderG, &QSlider::valueChanged, spinG, &QSpinBox::setValue);
    connect(sliderB, &QSlider::valueChanged, spinB, &QSpinBox::setValue);

    connect(spinH, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onHSVChanged);
    connect(spinS, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onHSVChanged);
    connect(spinV, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onHSVChanged);

    connect(spinX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onXYZChanged);
    connect(spinY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onXYZChanged);
    connect(spinZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onXYZChanged);

    connect(spinL, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onLabChanged);
    connect(spinA, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onLabChanged);
    connect(spinB_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onLabChanged);

    connect(comboIlluminant, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        if (isUpdating) return;
        model.setIlluminant(static_cast<Illuminant>(comboIlluminant->currentData().toInt()));
        updateAllViews();
    });

    connect(comboGamut, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        if (isUpdating) return;
        model.setGamutStrategy(static_cast<GamutStrategy>(comboGamut->currentData().toInt()));
        updateAllViews();
    });

    connect(btnPalette, &QPushButton::clicked, this, &MainWindow::openColorDialog);

    connect(btnPen, &QPushButton::clicked, this, [this]() {
        btnPen->setChecked(true);
        btnEraser->setChecked(false);
        editorController->setCurrentTool(ToolType::Pen);
    });

    connect(btnEraser, &QPushButton::clicked, this, [this]() {
        btnPen->setChecked(false);
        btnEraser->setChecked(true);
        editorController->setCurrentTool(ToolType::Eraser);
    });

    connect(spinWidth, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int width) {
        editorController->setPenWidth(width);
    });

    connect(btnSave, &QPushButton::clicked, this, &MainWindow::saveImage);
}

void MainWindow::onRGBChanged() {
    if (isUpdating) return;
    const RGBColor rgb{
        spinR->value() / 255.0,
        spinG->value() / 255.0,
        spinB->value() / 255.0
    };
    const HSVColor hsv = ColorModel::rgbToHsv(rgb);
    model.setHSV(hsv.h, hsv.s, hsv.v);
    updateAllViews();
}

void MainWindow::onHSVChanged() {
    if (isUpdating) return;
    model.setHSV(spinH->value(), spinS->value(), spinV->value());
    updateAllViews();
}

void MainWindow::onXYZChanged() {
    if (isUpdating) return;
    model.setXYZ(spinX->value(), spinY->value(), spinZ->value());
    updateAllViews();
}

void MainWindow::onLabChanged() {
    if (isUpdating) return;
    model.setLab(spinL->value(), spinA->value(), spinB_->value());
    updateAllViews();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if (watched == colorPreview && event->type() == QEvent::MouseButtonPress) {
        openColorDialog();
        return true;
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::openColorDialog() {
    ColorPickerDialog dialog(model.getQColor(), this);
    if (dialog.exec() != QDialog::Accepted)
        return;
    const RGBColor rgb = dialog.getRgb();
    const HSVColor hsv = ColorModel::rgbToHsv(rgb);
    model.setHSV(hsv.h, hsv.s, hsv.v);
    updateAllViews();
}

void MainWindow::saveImage() {
    const QString filePath = QFileDialog::getSaveFileName(
        this,
        "Сохранить изображение",
        "",
        "PNG Image (*.png);;JPEG Image (*.jpg *.jpeg);;BMP Image (*.bmp)"
        );

    if (filePath.isEmpty()) return;

    if (canvasModel.saveToFile(filePath)) {
        QMessageBox::information(this, "Успех", "Изображение успешно сохранено!");
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить изображение.");
    }
}

void MainWindow::updateGradients() {
    const int r = spinR->value();
    const int g = spinG->value();
    const int b = spinB->value();
    sliderR->setGradient(QColor(0, g, b), QColor(255, g, b));
    sliderG->setGradient(QColor(r, 0, b), QColor(r, 255, b));
    sliderB->setGradient(QColor(r, g, 0), QColor(r, g, 255));
}

void MainWindow::updateAllViews() {
    isUpdating = true;

    const RGBColor rgb = model.getRGB();
    const int ri = std::lround(rgb.r * 255.0);
    const int gi = std::lround(rgb.g * 255.0);
    const int bi = std::lround(rgb.b * 255.0);
    spinR->setValue(ri);
    spinG->setValue(gi);
    spinB->setValue(bi);
    sliderR->setValue(ri);
    sliderG->setValue(gi);
    sliderB->setValue(bi);

    const HSVColor hsv = model.getHSV();
    spinH->setValue(hsv.h);
    spinS->setValue(hsv.s);
    spinV->setValue(hsv.v);

    const XYZColor white = ColorModel::whitePoint(model.getIlluminant());
    spinX->setRange(0.0, std::max(1.0, std::ceil(white.x)));
    spinZ->setRange(0.0, std::max(1.0, std::ceil(white.z)));

    const XYZColor xyz = model.getXYZ();
    spinX->setValue(xyz.x);
    spinY->setValue(xyz.y);
    spinZ->setValue(xyz.z);

    const LabColor lab = model.getLab();
    spinL->setValue(lab.l);
    spinA->setValue(lab.a);
    spinB_->setValue(lab.b);

    comboIlluminant->setCurrentIndex(static_cast<int>(model.getIlluminant()));
    comboGamut->setCurrentIndex(static_cast<int>(model.getGamutStrategy()));
    gamutWarning->setVisible(model.isCurrentOutOfGamut());

    const QColor activeColor = model.getQColor();
    QPalette pal = colorPreview->palette();
    pal.setColor(QPalette::Window, activeColor);
    colorPreview->setPalette(pal);

    editorController->setPrimaryColor(activeColor);

    updateGradients();

    isUpdating = false;
}