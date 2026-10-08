#ifndef IMAGE_VIEW_H
#define IMAGE_VIEW_H

#include <string>
#include <vector>

#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Widget.H>

#include "core/image_buffer.h"

class ImageView : public Fl_Widget {
public:
    ImageView(int x, int y, int w, int h, const char* label_text);

    void set_image(const ImageBuffer* image);
    void set_caption(const char* text);
    void draw() FL_OVERRIDE;

private:
    std::vector<unsigned char> pixels;
    int image_width;
    int image_height;
    int image_channels;
    Fl_RGB_Image* fl_image;
    std::string caption;
};

#endif
