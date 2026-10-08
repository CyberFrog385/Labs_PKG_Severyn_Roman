#include "ui/image_view.h"

#include <cstddef>
#include <FL/fl_draw.H>

ImageView::ImageView(int x, int y, int w, int h, const char* label_text)
    : Fl_Widget(x, y, w, h),
      image_width(0),
      image_height(0),
      image_channels(0),
      fl_image(NULL),
      caption(label_text ? label_text : "") {
}

void ImageView::set_image(const ImageBuffer* image) {
    if (fl_image) {
        delete fl_image;
        fl_image = NULL;
    }
    pixels.clear();
    image_width = 0;
    image_height = 0;
    image_channels = 0;
    if (image && !image->empty()) {
        image_width = image->width();
        image_height = image->height();
        image_channels = image->channels();
        pixels.resize(static_cast<size_t>(image->row_stride()) * image_height);
        for (int y = 0; y < image_height; ++y) {
            const unsigned char* src = image->row(y);
            unsigned char* dst = &pixels[static_cast<size_t>(y) * image->row_stride()];
            for (int i = 0; i < image_width * image_channels; ++i) {
                dst[i] = src[i];
            }
        }
        fl_image = new Fl_RGB_Image(&pixels[0],
                                    image_width,
                                    image_height,
                                    image_channels,
                                    image->row_stride());
    }
    redraw();
}

void ImageView::set_caption(const char* text) {
    caption = text ? text : "";
    redraw();
}

void ImageView::draw() {
    fl_color(FL_BLACK);
    fl_rect(x(), y(), w(), h());
    fl_color(fl_rgb_color(32, 32, 32));
    fl_rectf(x() + 1, y() + 1, w() - 2, h() - 2);

    if (fl_image && image_width > 0 && image_height > 0) {
        double scale_x = static_cast<double>(w() - 4) / image_width;
        double scale_y = static_cast<double>(h() - 4) / image_height;
        double scale = scale_x < scale_y ? scale_x : scale_y;
        int draw_w = static_cast<int>(image_width * scale);
        int draw_h = static_cast<int>(image_height * scale);
        if (draw_w < 1) draw_w = 1;
        if (draw_h < 1) draw_h = 1;
        int draw_x = x() + (w() - draw_w) / 2;
        int draw_y = y() + (h() - draw_h) / 2;
        fl_push_clip(x() + 1, y() + 1, w() - 2, h() - 2);
        fl_image->draw(draw_x, draw_y, draw_w, draw_h);
        fl_pop_clip();
    } else {
        fl_color(fl_rgb_color(160, 160, 160));
        fl_font(FL_HELVETICA, 16);
        const char* text = "Нет изображения";
        int text_w = 0;
        int text_h = 0;
        fl_measure(text, text_w, text_h);
        fl_draw(text, x() + (w() - text_w) / 2, y() + h() / 2);
    }

    if (!caption.empty()) {
        fl_color(fl_rgb_color(240, 240, 240));
        fl_font(FL_HELVETICA_BOLD, 13);
        int text_w = 0;
        int text_h = 0;
        fl_measure(caption.c_str(), text_w, text_h);
        fl_color(FL_BLACK);
        fl_rectf(x() + 6, y() + 6, text_w + 8, text_h + 4);
        fl_color(fl_rgb_color(240, 240, 240));
        fl_draw(caption.c_str(), x() + 10, y() + 8 + text_h);
    }
}
