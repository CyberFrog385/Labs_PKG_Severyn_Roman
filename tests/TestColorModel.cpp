#include <QtTest>
#include <cmath>
#include "ColorModel.h"

class TestColorModel : public QObject {
    Q_OBJECT

private slots:
    void redToXyz();
    void redToLabD65();
    void whiteToLabCorrectness();
    void roundTripHsv();
    void roundTripLab();
    void illuminantAdaptation();
    void gamutClipping();
    void gamutScaling();
    void setterRoundTrip();
    void outOfGamutDetection();
};

static bool approx(double a, double b, double tolerance = 1e-3)
{
    return std::abs(a - b) <= tolerance;
}

void TestColorModel::redToXyz()
{
    const XYZColor xyz = ColorModel::rgbToXyz({1.0, 0.0, 0.0}, Illuminant::D65);
    QVERIFY(approx(xyz.x, 41.245, 1e-2));
    QVERIFY(approx(xyz.y, 21.267, 1e-2));
    QVERIFY(approx(xyz.z, 1.934, 1e-2));
}

void TestColorModel::redToLabD65()
{
    const XYZColor xyz = ColorModel::rgbToXyz({1.0, 0.0, 0.0}, Illuminant::D65);
    const LabColor lab = ColorModel::xyzToLab(xyz, Illuminant::D65);
    QVERIFY(approx(lab.l, 53.24, 0.5));
    QVERIFY(approx(lab.a, 80.09, 0.5));
    QVERIFY(approx(lab.b, 67.20, 0.5));
}

void TestColorModel::whiteToLabCorrectness()
{
    const XYZColor d50 = ColorModel::whitePoint(Illuminant::D50);
    const LabColor lab = ColorModel::xyzToLab(d50, Illuminant::D50);
    QVERIFY(approx(lab.l, 100.0, 1e-6));
    QVERIFY(approx(lab.a, 0.0, 1e-6));
    QVERIFY(approx(lab.b, 0.0, 1e-6));

    const LabColor labE = ColorModel::xyzToLab(ColorModel::whitePoint(Illuminant::E), Illuminant::E);
    QVERIFY(approx(labE.l, 100.0, 1e-6));
    QVERIFY(approx(labE.a, 0.0, 1e-6));
    QVERIFY(approx(labE.b, 0.0, 1e-6));
}

void TestColorModel::roundTripHsv()
{
    const RGBColor samples[] = {
        {1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {0.0, 0.0, 1.0},
        {1.0, 1.0, 1.0},
        {0.0, 0.0, 0.0},
        {0.3, 0.6, 0.9}
    };
    for (const RGBColor &sample : samples) {
        const RGBColor back = ColorModel::hsvToRgb(ColorModel::rgbToHsv(sample));
        QVERIFY(approx(back.r, sample.r, 1e-4));
        QVERIFY(approx(back.g, sample.g, 1e-4));
        QVERIFY(approx(back.b, sample.b, 1e-4));
    }
}

void TestColorModel::roundTripLab()
{
    const Illuminant illums[] = {Illuminant::D65, Illuminant::D50, Illuminant::E};
    for (Illuminant illum : illums) {
        const XYZColor xyz = ColorModel::rgbToXyz({0.4, 0.7, 0.2}, illum);
        const LabColor lab = ColorModel::xyzToLab(xyz, illum);
        const XYZColor back = ColorModel::labToXyz(lab, illum);
        QVERIFY(approx(back.x, xyz.x, 1e-4));
        QVERIFY(approx(back.y, xyz.y, 1e-4));
        QVERIFY(approx(back.z, xyz.z, 1e-4));
    }
}

void TestColorModel::illuminantAdaptation()
{
    const XYZColor whiteE = ColorModel::rgbToXyz({1.0, 1.0, 1.0}, Illuminant::E);
    QVERIFY(approx(whiteE.x, 100.0, 1e-2));
    QVERIFY(approx(whiteE.y, 100.0, 1e-2));
    QVERIFY(approx(whiteE.z, 100.0, 1e-2));

    const XYZColor redD65 = ColorModel::rgbToXyz({1.0, 0.0, 0.0}, Illuminant::D65);
    const XYZColor redD50 = ColorModel::rgbToXyz({1.0, 0.0, 0.0}, Illuminant::D50);
    QVERIFY(std::abs(redD65.x - redD50.x) > 0.1);
}

void TestColorModel::gamutClipping()
{
    const LabColor out{55.0, 120.0, 60.0};
    const XYZColor xyz = ColorModel::labToXyz(out, Illuminant::D65);
    const RGBColor rgb = ColorModel::xyzToRgb(xyz, Illuminant::D65, GamutStrategy::Clipping);
    QVERIFY(rgb.r >= 0.0 && rgb.r <= 1.0);
    QVERIFY(rgb.g >= 0.0 && rgb.g <= 1.0);
    QVERIFY(rgb.b >= 0.0 && rgb.b <= 1.0);
    const bool atBoundary = rgb.r <= 1e-12 || rgb.g <= 1e-12 || rgb.b <= 1e-12
                         || rgb.r >= 1.0 - 1e-12 || rgb.g >= 1.0 - 1e-12 || rgb.b >= 1.0 - 1e-12;
    QVERIFY(atBoundary);
}

void TestColorModel::gamutScaling()
{
    const LabColor out{55.0, 120.0, 60.0};
    const XYZColor xyz = ColorModel::labToXyz(out, Illuminant::D65);
    const RGBColor rgb = ColorModel::xyzToRgb(xyz, Illuminant::D65, GamutStrategy::Scaling);
    QVERIFY(rgb.r >= 0.0 && rgb.r <= 1.0);
    QVERIFY(rgb.g >= 0.0 && rgb.g <= 1.0);
    QVERIFY(rgb.b >= 0.0 && rgb.b <= 1.0);
    const double mx = std::max({rgb.r, rgb.g, rgb.b});
    QVERIFY(mx >= 1.0 - 1e-9);
}

void TestColorModel::setterRoundTrip()
{
    ColorModel model;
    model.setHSV(0.0, 1.0, 1.0);
    const QColor c = model.getQColor();
    QVERIFY(approx(c.redF(), 1.0, 1e-4));
    QVERIFY(approx(c.greenF(), 0.0, 1e-4));
    QVERIFY(approx(c.blueF(), 0.0, 1e-4));
}

void TestColorModel::outOfGamutDetection()
{
    QVERIFY(ColorModel::isOutOfGamut({200.0, 100.0, 100.0}, Illuminant::D65));
    QVERIFY(!ColorModel::isOutOfGamut({41.245, 21.267, 1.934}, Illuminant::D65));

    ColorModel model;
    model.setXYZ(200.0, 100.0, 100.0);
    QVERIFY(model.isCurrentOutOfGamut());
    model.setHSV(0.0, 1.0, 1.0);
    QVERIFY(!model.isCurrentOutOfGamut());
    model.setLab(55.0, 120.0, 60.0);
    QVERIFY(model.isCurrentOutOfGamut());
}

QTEST_MAIN(TestColorModel)
#include "TestColorModel.moc"