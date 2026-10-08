#ifndef IMAGE_IO_H
#define IMAGE_IO_H

#include <string>

#include "core/image_buffer.h"

ImageBuffer load_image(const std::string& path, std::string* error);
bool save_png(const ImageBuffer& image, const std::string& path, std::string* error);

#endif
