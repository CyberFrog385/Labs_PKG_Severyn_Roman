#ifndef COLORMODEL_H
#define COLORMODEL_H

#include <QColor>

struct HSVColor { double h; double s; double v; };
struct RGBColor { double r; double g; double b; };
struct XYZColor { double x; double y; double z; };
struct LabColor { double l; double a; double b; };

enum class Illuminant { D65, D50, E };
enum class GamutStrategy { Clipping, Scaling };

class ColorModel {
public:
    ColorModel();

    void setHSV(double h, double s, double v);
    void setXYZ(double x, double y, double z);
    void setLab(double l, double a, double b);

    void setIlluminant(Illuminant illuminant);
    Illuminant getIlluminant() const { return currentIlluminant; }

    void setGamutStrategy(GamutStrategy strategy);
    GamutStrategy getGamutStrategy() const { return gamutStrategy; }

    HSVColor getHSV() const { return currentHSV; }
    XYZColor getXYZ() const { return currentXYZ; }
    LabColor getLab() const { return currentLab; }
    RGBColor getRGB() const { return hsvToRgb(currentHSV); }
    QColor getQColor() const;

    static XYZColor whitePoint(Illuminant illuminant) noexcept;

    static RGBColor hsvToRgb(HSVColor hsv) noexcept;
    static HSVColor rgbToHsv(RGBColor rgb) noexcept;

    static XYZColor rgbToXyz(RGBColor rgb, Illuminant illuminant) noexcept;
    static RGBColor xyzToRgb(XYZColor xyz, Illuminant illuminant, GamutStrategy strategy) noexcept;

    static bool isOutOfGamut(XYZColor xyz, Illuminant illuminant) noexcept;
    bool isCurrentOutOfGamut() const { return outOfGamut; }

    static LabColor xyzToLab(XYZColor xyz, Illuminant illuminant) noexcept;
    static XYZColor labToXyz(LabColor lab, Illuminant illuminant) noexcept;

private:
    enum class LastEdited { HSV, XYZ, Lab };

    Illuminant currentIlluminant = Illuminant::D65;
    GamutStrategy gamutStrategy = GamutStrategy::Clipping;
    LastEdited lastEdited = LastEdited::HSV;
    bool outOfGamut = false;

    HSVColor currentHSV{0.0, 0.0, 0.0};
    XYZColor currentXYZ{0.0, 0.0, 0.0};
    LabColor currentLab{0.0, 0.0, 0.0};

    XYZColor lastEditedXyz{0.0, 0.0, 0.0};
    LabColor lastEditedLab{0.0, 0.0, 0.0};

    static RGBColor xyzToRgbLinear(XYZColor xyz, Illuminant illuminant) noexcept;
    static bool isOutOfGamutLinear(const RGBColor &linear) noexcept;
    static void getMatrices(Illuminant illuminant, double rgbToXyzM[3][3], double xyzToRgbM[3][3]);
};

#endif