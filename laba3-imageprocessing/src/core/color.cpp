#include "core/color.h"

#include <cmath>

HsvColor rgb_to_hsv(unsigned char r, unsigned char g, unsigned char b) {
    double rd = r / 255.0;
    double gd = g / 255.0;
    double bd = b / 255.0;
    double max_value = rd;
    if (gd > max_value) max_value = gd;
    if (bd > max_value) max_value = bd;
    double min_value = rd;
    if (gd < min_value) min_value = gd;
    if (bd < min_value) min_value = bd;
    double diff = max_value - min_value;

    HsvColor hsv;
    hsv.v = max_value;
    hsv.s = (max_value <= 0.0) ? 0.0 : diff / max_value;
    hsv.h = 0.0;
    if (diff > 0.0) {
        if (max_value == rd) {
            hsv.h = 60.0 * fmod((gd - bd) / diff, 6.0);
        } else if (max_value == gd) {
            hsv.h = 60.0 * ((bd - rd) / diff + 2.0);
        } else {
            hsv.h = 60.0 * ((rd - gd) / diff + 4.0);
        }
        if (hsv.h < 0.0) hsv.h += 360.0;
    }
    return hsv;
}

void hsv_to_rgb(const HsvColor& hsv, unsigned char& r, unsigned char& g, unsigned char& b) {
    double v = hsv.v;
    if (v < 0.0) v = 0.0;
    if (v > 1.0) v = 1.0;
    double s = hsv.s;
    if (s < 0.0) s = 0.0;
    if (s > 1.0) s = 1.0;
    double h = hsv.h;
    while (h < 0.0) h += 360.0;
    while (h >= 360.0) h -= 360.0;

    double c = v * s;
    double x = c * (1.0 - std::fabs(fmod(h / 60.0, 2.0) - 1.0));
    double m = v - c;
    double rd = 0.0;
    double gd = 0.0;
    double bd = 0.0;
    if (h < 60.0) {
        rd = c; gd = x; bd = 0.0;
    } else if (h < 120.0) {
        rd = x; gd = c; bd = 0.0;
    } else if (h < 180.0) {
        rd = 0.0; gd = c; bd = x;
    } else if (h < 240.0) {
        rd = 0.0; gd = x; bd = c;
    } else if (h < 300.0) {
        rd = x; gd = 0.0; bd = c;
    } else {
        rd = c; gd = 0.0; bd = x;
    }
    r = static_cast<unsigned char>((rd + m) * 255.0 + 0.5);
    g = static_cast<unsigned char>((gd + m) * 255.0 + 0.5);
    b = static_cast<unsigned char>((bd + m) * 255.0 + 0.5);
}
