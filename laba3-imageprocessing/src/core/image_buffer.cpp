#include "core/image_buffer.h"

#include <cstddef>

ImageBuffer::ImageBuffer()
    : w(0), h(0), ch(0), stride(0) {
}

ImageBuffer::ImageBuffer(int width, int height, int channels)
    : w(width), h(height), ch(channels), stride(0) {
    if (w < 0) w = 0;
    if (h < 0) h = 0;
    if (ch < 1) ch = 1;
    if (ch > 4) ch = 4;
    stride = (w * ch + 3) / 4 * 4;
    data.assign(static_cast<size_t>(stride) * h, 0);
}

bool ImageBuffer::empty() const {
    return w <= 0 || h <= 0;
}

int ImageBuffer::width() const {
    return w;
}

int ImageBuffer::height() const {
    return h;
}

int ImageBuffer::channels() const {
    return ch;
}

int ImageBuffer::row_stride() const {
    return stride;
}

unsigned char* ImageBuffer::row(int y) {
    return &data[static_cast<size_t>(clamp_coord(y, h)) * stride];
}

const unsigned char* ImageBuffer::row(int y) const {
    return &data[static_cast<size_t>(clamp_coord(y, h)) * stride];
}

unsigned char ImageBuffer::get(int x, int y, int channel) const {
    int cx = clamp_coord(x, w);
    int cy = clamp_coord(y, h);
    int cc = clamp_int(channel, 0, ch - 1);
    return data[static_cast<size_t>(cy) * stride + cx * ch + cc];
}

void ImageBuffer::set(int x, int y, int channel, unsigned char value) {
    int cx = clamp_coord(x, w);
    int cy = clamp_coord(y, h);
    int cc = clamp_int(channel, 0, ch - 1);
    data[static_cast<size_t>(cy) * stride + cx * ch + cc] = value;
}

int ImageBuffer::clamp_int(int value, int low, int high) {
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

int ImageBuffer::clamp_coord(int value, int size) {
    if (size <= 0) return 0;
    if (value < 0) return 0;
    if (value >= size) return size - 1;
    return value;
}

int ImageBuffer::reflect_coord(int value, int size) {
    if (size <= 1) return 0;
    while (value < 0 || value >= size) {
        if (value < 0) value = -value;
        if (value >= size) value = 2 * size - 2 - value;
    }
    return value;
}
