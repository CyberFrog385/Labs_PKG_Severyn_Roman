#include "ColorModel.h"

#include <algorithm>
#include <cmath>

namespace {

void matVec(double out[3], const double m[3][3], const double v[3])
{
    for (int i = 0; i < 3; ++i)
        out[i] = m[i][0] * v[0] + m[i][1] * v[1] + m[i][2] * v[2];
}

void matMul(double out[3][3], const double a[3][3], const double b[3][3])
{
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            out[i][j] = a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j];
        }
    }
}

void matInv3(double out[3][3], const double m[3][3])
{
    const double d = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
                   - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
                   + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
    const double f = 1.0 / d;
    out[0][0] =  (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * f;
    out[0][1] = -(m[0][1] * m[2][2] - m[0][2] * m[2][1]) * f;
    out[0][2] =  (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * f;
    out[1][0] = -(m[1][0] * m[2][2] - m[1][2] * m[2][0]) * f;
    out[1][1] =  (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * f;
    out[1][2] = -(m[0][0] * m[1][2] - m[0][2] * m[1][0]) * f;
    out[2][0] =  (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * f;
    out[2][1] = -(m[0][0] * m[2][1] - m[0][1] * m[2][0]) * f;
    out[2][2] =  (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * f;
}

double srgbToLinear(double c)
{
    return c >= 0.04045 ? std::pow((c + 0.055) / 1.055, 2.4) : c / 12.92;
}

double linearToSrgb(double c)
{
    return c >= 0.0031308 ? 1.055 * std::pow(c, 1.0 / 2.4) - 0.055 : 12.92 * c;
}

double d65f(double t)
{
    return t >= 0.008856 ? std::pow(t, 1.0 / 3.0) : 7.787 * t + 16.0 / 116.0;
}

double d65fInv(double t)
{
    const double t3 = t * t * t;
    return t3 >= 0.008856 ? t3 : (t - 16.0 / 116.0) / 7.787;
}

}

ColorModel::ColorModel()
{
    setHSV(0.0, 0.0, 0.0);
}

void ColorModel::setHSV(double h, double s, double v)
{
    lastEdited = LastEdited::HSV;
    currentHSV = {h, s, v};
    currentXYZ = rgbToXyz(hsvToRgb(currentHSV), currentIlluminant);
    currentLab = xyzToLab(currentXYZ, currentIlluminant);
    outOfGamut = false;
}

void ColorModel::setXYZ(double x, double y, double z)
{
    lastEdited = LastEdited::XYZ;
    currentXYZ = {x, y, z};
    lastEditedXyz = currentXYZ;
    const RGBColor linear = xyzToRgbLinear(currentXYZ, currentIlluminant);
    outOfGamut = isOutOfGamutLinear(linear);
    const RGBColor rgb = xyzToRgb(currentXYZ, currentIlluminant, gamutStrategy);
    currentHSV = rgbToHsv(rgb);
    currentLab = xyzToLab(currentXYZ, currentIlluminant);
}

void ColorModel::setLab(double l, double a, double b)
{
    lastEdited = LastEdited::Lab;
    currentLab = {l, a, b};
    lastEditedLab = currentLab;
    currentXYZ = labToXyz(currentLab, currentIlluminant);
    const RGBColor linear = xyzToRgbLinear(currentXYZ, currentIlluminant);
    outOfGamut = isOutOfGamutLinear(linear);
    currentHSV = rgbToHsv(xyzToRgb(currentXYZ, currentIlluminant, gamutStrategy));
}

void ColorModel::setIlluminant(Illuminant illuminant)
{
    if (illuminant == currentIlluminant)
        return;
    currentIlluminant = illuminant;
    switch (lastEdited) {
    case LastEdited::XYZ:
        setXYZ(lastEditedXyz.x, lastEditedXyz.y, lastEditedXyz.z);
        break;
    case LastEdited::Lab:
        setLab(lastEditedLab.l, lastEditedLab.a, lastEditedLab.b);
        break;
    case LastEdited::HSV:
    default:
        currentXYZ = rgbToXyz(hsvToRgb(currentHSV), currentIlluminant);
        currentLab = xyzToLab(currentXYZ, currentIlluminant);
        outOfGamut = false;
        break;
    }
}

void ColorModel::setGamutStrategy(GamutStrategy strategy)
{
    if (strategy == gamutStrategy)
        return;
    gamutStrategy = strategy;
    switch (lastEdited) {
    case LastEdited::XYZ:
        setXYZ(lastEditedXyz.x, lastEditedXyz.y, lastEditedXyz.z);
        break;
    case LastEdited::Lab:
        setLab(lastEditedLab.l, lastEditedLab.a, lastEditedLab.b);
        break;
    case LastEdited::HSV:
    default:
        break;
    }
}

QColor ColorModel::getQColor() const
{
    const RGBColor rgb = hsvToRgb(currentHSV);
    return QColor::fromRgbF(
        std::clamp(rgb.r, 0.0, 1.0),
        std::clamp(rgb.g, 0.0, 1.0),
        std::clamp(rgb.b, 0.0, 1.0));
}

XYZColor ColorModel::whitePoint(Illuminant illuminant) noexcept
{
    switch (illuminant) {
    case Illuminant::D65:
        return {95.047, 100.0, 108.883};
    case Illuminant::D50:
        return {96.422, 100.0, 82.521};
    case Illuminant::E:
    default:
        return {100.0, 100.0, 100.0};
    }
}

void ColorModel::getMatrices(Illuminant illuminant, double rgbToXyzM[3][3], double xyzToRgbM[3][3])
{
    const double bradford[3][3] = {
        {0.8951, 0.2664, -0.1614},
        {-0.7502, 1.7135, 0.0367},
        {0.0389, -0.0685, 1.0296}
    };
    const double bradfordInv[3][3] = {
        {0.9869929, -0.1470543, 0.1599627},
        {0.4323053, 0.5183603, 0.0492912},
        {-0.0085287, 0.0400428, 0.9684867}
    };

    const XYZColor ref = whitePoint(Illuminant::D65);
    const XYZColor target = whitePoint(illuminant);
    const double src[3] = {ref.x, ref.y, ref.z};
    const double tgt[3] = {target.x, target.y, target.z};

    double srcCone[3], dstCone[3];
    matVec(srcCone, bradford, src);
    matVec(dstCone, bradford, tgt);

    const double diag[3][3] = {
        {dstCone[0] / srcCone[0], 0.0, 0.0},
        {0.0, dstCone[1] / srcCone[1], 0.0},
        {0.0, 0.0, dstCone[2] / srcCone[2]}
    };

    double temp[3][3];
    matMul(temp, bradfordInv, diag);
    double adapt[3][3];
    matMul(adapt, temp, bradford);

    const double m65[3][3] = {
        {0.412453, 0.357580, 0.180423},
        {0.212671, 0.715160, 0.072169},
        {0.019340, 0.119193, 0.950227}
    };

    matMul(rgbToXyzM, adapt, m65);
    matInv3(xyzToRgbM, rgbToXyzM);
}

RGBColor ColorModel::hsvToRgb(HSVColor hsv) noexcept
{
    const double c = hsv.v * hsv.s;
    const double hPrime = hsv.h / 60.0;
    const double x = c * (1.0 - std::abs(std::fmod(hPrime, 2.0) - 1.0));
    const double m = hsv.v - c;

    double r1 = 0.0, g1 = 0.0, b1 = 0.0;
    if (hPrime < 1.0)      { r1 = c; g1 = x; }
    else if (hPrime < 2.0) { r1 = x; g1 = c; }
    else if (hPrime < 3.0) { g1 = c; b1 = x; }
    else if (hPrime < 4.0) { g1 = x; b1 = c; }
    else if (hPrime < 5.0) { r1 = x; b1 = c; }
    else                   { r1 = c; b1 = x; }

    return {r1 + m, g1 + m, b1 + m};
}

HSVColor ColorModel::rgbToHsv(RGBColor rgb) noexcept
{
    const double maxC = std::max({rgb.r, rgb.g, rgb.b});
    const double minC = std::min({rgb.r, rgb.g, rgb.b});
    const double delta = maxC - minC;

    double h = 0.0;
    if (delta > 1e-5) {
        if (maxC == rgb.r)      h = 60.0 * std::fmod((rgb.g - rgb.b) / delta, 6.0);
        else if (maxC == rgb.g) h = 60.0 * (((rgb.b - rgb.r) / delta) + 2.0);
        else                    h = 60.0 * (((rgb.r - rgb.g) / delta) + 4.0);
    }
    if (h < 0.0) h += 360.0;

    const double s = (maxC > 1e-5) ? (delta / maxC) : 0.0;
    return {h, s, maxC};
}

XYZColor ColorModel::rgbToXyz(RGBColor rgb, Illuminant illuminant) noexcept
{
    double m[3][3];
    double unused[3][3];
    getMatrices(illuminant, m, unused);

    const double Rn = srgbToLinear(rgb.r) * 100.0;
    const double Gn = srgbToLinear(rgb.g) * 100.0;
    const double Bn = srgbToLinear(rgb.b) * 100.0;

    return {
        m[0][0] * Rn + m[0][1] * Gn + m[0][2] * Bn,
        m[1][0] * Rn + m[1][1] * Gn + m[1][2] * Bn,
        m[2][0] * Rn + m[2][1] * Gn + m[2][2] * Bn
    };
}

RGBColor ColorModel::xyzToRgb(XYZColor xyz, Illuminant illuminant, GamutStrategy strategy) noexcept
{
    RGBColor linear = xyzToRgbLinear(xyz, illuminant);
    double rLin = linear.r;
    double gLin = linear.g;
    double bLin = linear.b;

    if (strategy == GamutStrategy::Scaling) {
        const double mn = std::min({rLin, gLin, bLin});
        if (mn < 0.0) {
            rLin -= mn;
            gLin -= mn;
            bLin -= mn;
        }
        const double mx = std::max({rLin, gLin, bLin});
        if (mx > 1.0) {
            const double scale = 1.0 / mx;
            rLin *= scale;
            gLin *= scale;
            bLin *= scale;
        }
    }

    return {
        std::clamp(linearToSrgb(rLin), 0.0, 1.0),
        std::clamp(linearToSrgb(gLin), 0.0, 1.0),
        std::clamp(linearToSrgb(bLin), 0.0, 1.0)
    };
}

RGBColor ColorModel::xyzToRgbLinear(XYZColor xyz, Illuminant illuminant) noexcept
{
    double unused[3][3];
    double inv[3][3];
    getMatrices(illuminant, unused, inv);

    const double x = xyz.x / 100.0;
    const double y = xyz.y / 100.0;
    const double z = xyz.z / 100.0;

    return {
        inv[0][0] * x + inv[0][1] * y + inv[0][2] * z,
        inv[1][0] * x + inv[1][1] * y + inv[1][2] * z,
        inv[2][0] * x + inv[2][1] * y + inv[2][2] * z
    };
}

bool ColorModel::isOutOfGamutLinear(const RGBColor &linear) noexcept
{
    return linear.r < 0.0 || linear.r > 1.0
        || linear.g < 0.0 || linear.g > 1.0
        || linear.b < 0.0 || linear.b > 1.0;
}

bool ColorModel::isOutOfGamut(XYZColor xyz, Illuminant illuminant) noexcept
{
    return isOutOfGamutLinear(xyzToRgbLinear(xyz, illuminant));
}

LabColor ColorModel::xyzToLab(XYZColor xyz, Illuminant illuminant) noexcept
{
    const XYZColor white = whitePoint(illuminant);

    const double fX = d65f(xyz.x / white.x);
    const double fY = d65f(xyz.y / white.y);
    const double fZ = d65f(xyz.z / white.z);

    return {
        116.0 * fY - 16.0,
        500.0 * (fX - fY),
        200.0 * (fY - fZ)
    };
}

XYZColor ColorModel::labToXyz(LabColor lab, Illuminant illuminant) noexcept
{
    const XYZColor white = whitePoint(illuminant);

    const double pY = (lab.l + 16.0) / 116.0;
    const double pX = lab.a / 500.0 + pY;
    const double pZ = pY - lab.b / 200.0;

    return {
        d65fInv(pX) * white.x,
        d65fInv(pY) * white.y,
        d65fInv(pZ) * white.z
    };
}