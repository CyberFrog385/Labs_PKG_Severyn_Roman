#ifndef COLOR_H
#define COLOR_H

struct HsvColor {
    double h;
    double s;
    double v;
};

HsvColor rgb_to_hsv(unsigned char r, unsigned char g, unsigned char b);
void hsv_to_rgb(const HsvColor& hsv, unsigned char& r, unsigned char& g, unsigned char& b);

#endif
