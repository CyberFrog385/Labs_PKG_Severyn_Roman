#include "core/image_io.h"

#include <cstddef>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Shared_Image.H>

ImageBuffer load_image(const std::string& path, std::string* error) {
    fl_register_images();
    Fl_Shared_Image* shared = Fl_Shared_Image::get(path.c_str());
    if (!shared) {
        if (error) *error = "Не удалось открыть файл изображения: " + path;
        return ImageBuffer();
    }
    int w = shared->w();
    int h = shared->h();
    int d = shared->d();
    int ld = shared->ld();
    if (ld == 0) ld = w * d;
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(shared->data()[0]);

    int out_channels = (d == 1 || d == 2) ? 1 : 3;
    ImageBuffer out(w, h, out_channels);
    for (int y = 0; y < h; ++y) {
        const unsigned char* src = bytes + static_cast<size_t>(y) * ld;
        unsigned char* dst = out.row(y);
        if (out_channels == 1) {
            for (int x = 0; x < w; ++x) {
                dst[x] = src[x * d];
            }
        } else {
            for (int x = 0; x < w; ++x) {
                dst[x * 3] = src[x * d];
                dst[x * 3 + 1] = src[x * d + 1];
                dst[x * 3 + 2] = src[x * d + 2];
            }
        }
    }
    shared->release();
    return out;
}

bool save_png(const ImageBuffer& image, const std::string& path, std::string* error) {
    if (image.empty()) {
        if (error) *error = "Изображение пустое, сохранять нечего";
        return false;
    }
    int code = fl_write_png(path.c_str(),
                            image.row(0),
                            image.width(),
                            image.height(),
                            image.channels(),
                            image.row_stride());
    if (code != 0) {
        if (error) *error = "Ошибка сохранения PNG (код " + std::to_string(code) + "): " + path;
        return false;
    }
    return true;
}
