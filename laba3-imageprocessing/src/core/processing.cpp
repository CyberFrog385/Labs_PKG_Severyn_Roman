#include "core/processing.h"

#include <cmath>

#include "core/color.h"

namespace {

void report_row(Progress* progress) {
    if (progress) {
        progress->value.fetch_add(1);
    }
}

ImageBuffer clone_shape(const ImageBuffer& image) {
    return ImageBuffer(image.width(), image.height(), image.channels());
}

unsigned char clamp_byte(double value) {
    if (value < 0.0) return 0;
    if (value > 255.0) return 255;
    return static_cast<unsigned char>(value + 0.5);
}

}

void compute_histogram(const ImageBuffer& image, int channel, int histogram[256]) {
    for (int i = 0; i < 256; ++i) {
        histogram[i] = 0;
    }
    if (image.empty()) return;
    int h = image.height();
    int w = image.width();
    int ch = image.channels();
    for (int y = 0; y < h; ++y) {
        const unsigned char* row = image.row(y);
        if (channel >= 0 && channel < ch) {
            for (int x = 0; x < w; ++x) {
                histogram[row[x * ch + channel]] += 1;
            }
        } else {
            for (int x = 0; x < w; ++x) {
                double value = 0.299 * row[x * ch] + 0.587 * row[x * ch + 1] + 0.114 * row[x * ch + 2];
                histogram[static_cast<int>(value + 0.5)] += 1;
            }
        }
    }
}

void compute_statistics(const ImageBuffer& image, int channel, double out[4]) {
    out[0] = 255;
    out[1] = 0;
    out[2] = 0;
    out[3] = 0;
    if (image.empty()) return;
    int h = image.height();
    int w = image.width();
    int ch = image.channels();
    int count = w * h;
    double sum = 0.0;
    double sum_sq = 0.0;
    int lo = 255;
    int hi = 0;
    if (channel >= 0 && channel < ch) {
        for (int y = 0; y < h; ++y) {
            const unsigned char* row = image.row(y);
            for (int x = 0; x < w; ++x) {
                int v = row[x * ch + channel];
                sum += v;
                sum_sq += v * v;
                if (v < lo) lo = v;
                if (v > hi) hi = v;
            }
        }
    } else {
        for (int y = 0; y < h; ++y) {
            const unsigned char* row = image.row(y);
            for (int x = 0; x < w; ++x) {
                int v = static_cast<int>(0.299 * row[x * ch] + 0.587 * row[x * ch + 1] + 0.114 * row[x * ch + 2] + 0.5);
                sum += v;
                sum_sq += v * v;
                if (v < lo) lo = v;
                if (v > hi) hi = v;
            }
        }
    }
    double mean = sum / count;
    double variance = sum_sq / count - mean * mean;
    if (variance < 0.0) variance = 0.0;
    out[0] = lo;
    out[1] = hi;
    out[2] = mean;
    out[3] = std::sqrt(variance);
}

ImageBuffer linear_contrast_auto(const ImageBuffer& image, Progress* progress) {
    if (image.empty()) return clone_shape(image);
    int h = image.height();
    int w = image.width();
    int ch = image.channels();
    int lo = 255;
    int hi = 0;
    for (int y = 0; y < h; ++y) {
        const unsigned char* row = image.row(y);
        for (int i = 0; i < w * ch; ++i) {
            int v = row[i];
            if (v < lo) lo = v;
            if (v > hi) hi = v;
        }
    }
    ImageBuffer out(w, h, ch);
    if (progress) progress->reset(h);
    double scale = 1.0;
    if (hi > lo) scale = 255.0 / (hi - lo);
    double offset = -lo;
    parallel_rows(h, max_threads(), [&](int y0, int y1) {
        for (int y = y0; y < y1; ++y) {
            const unsigned char* src = image.row(y);
            unsigned char* dst = out.row(y);
            for (int i = 0; i < w * ch; ++i) {
                dst[i] = clamp_byte((src[i] + offset) * scale);
            }
            report_row(progress);
        }
    });
    return out;
}

ImageBuffer linear_contrast_manual(const ImageBuffer& image, int in_min, int in_max, Progress* progress) {
    if (image.empty()) return clone_shape(image);
    int h = image.height();
    int w = image.width();
    int ch = image.channels();
    ImageBuffer out(w, h, ch);
    if (progress) progress->reset(h);
    if (in_max <= in_min) {
        for (int y = 0; y < h; ++y) {
            const unsigned char* src = image.row(y);
            unsigned char* dst = out.row(y);
            for (int i = 0; i < w * ch; ++i) {
                dst[i] = src[i];
            }
            report_row(progress);
        }
        return out;
    }
    double scale = 255.0 / (in_max - in_min);
    double offset = -in_min;
    parallel_rows(h, max_threads(), [&](int y0, int y1) {
        for (int y = y0; y < y1; ++y) {
            const unsigned char* src = image.row(y);
            unsigned char* dst = out.row(y);
            for (int i = 0; i < w * ch; ++i) {
                dst[i] = clamp_byte((src[i] + offset) * scale);
            }
            report_row(progress);
        }
    });
    return out;
}

ImageBuffer equalize_channel(const ImageBuffer& image, Progress* progress) {
    if (image.empty()) return clone_shape(image);
    int h = image.height();
    int w = image.width();
    int ch = image.channels();
    ImageBuffer out(w, h, ch);
    int total = w * h;
    for (int c = 0; c < ch; ++c) {
        int histogram[256];
        for (int i = 0; i < 256; ++i) histogram[i] = 0;
        for (int y = 0; y < h; ++y) {
            const unsigned char* row = image.row(y);
            for (int x = 0; x < w; ++x) {
                histogram[row[x * ch + c]] += 1;
            }
        }
        int cdf = 0;
        int cdf_min = -1;
        for (int i = 0; i < 256; ++i) {
            cdf += histogram[i];
            if (cdf_min < 0 && histogram[i] > 0) {
                cdf_min = cdf;
            }
        }
        unsigned char map[256];
        int denom = total - cdf_min;
        if (denom <= 0) {
            for (int i = 0; i < 256; ++i) {
                map[i] = static_cast<unsigned char>(i);
            }
        } else {
            for (int i = 0; i < 256; ++i) {
                int count = 0;
                for (int k = 0; k <= i; ++k) count += histogram[k];
                int value = (count - cdf_min) * 255 / denom;
                if (value < 0) value = 0;
                if (value > 255) value = 255;
                map[i] = static_cast<unsigned char>(value);
            }
        }
        parallel_rows(h, max_threads(), [&](int y0, int y1) {
            for (int y = y0; y < y1; ++y) {
                const unsigned char* src = image.row(y);
                unsigned char* dst = out.row(y);
                for (int x = 0; x < w; ++x) {
                    dst[x * ch + c] = map[src[x * ch + c]];
                }
                if (c == ch - 1) report_row(progress);
            }
        });
    }
    return out;
}

ImageBuffer equalize_rgb(const ImageBuffer& image, Progress* progress) {
    if (progress) progress->reset(image.height());
    return equalize_channel(image, progress);
}

ImageBuffer equalize_hsv(const ImageBuffer& image, Progress* progress) {
    if (image.empty()) return clone_shape(image);
    int h = image.height();
    int w = image.width();
    int ch = image.channels();
    if (ch == 1) {
        return equalize_channel(image, progress);
    }
    ImageBuffer value_plane(w, h, 1);
    if (progress) progress->reset(h * 3);
    parallel_rows(h, max_threads(), [&](int y0, int y1) {
        for (int y = y0; y < y1; ++y) {
            const unsigned char* src = image.row(y);
            unsigned char* dst = value_plane.row(y);
            for (int x = 0; x < w; ++x) {
                HsvColor hsv = rgb_to_hsv(src[x * ch], src[x * ch + 1], src[x * ch + 2]);
                dst[x] = clamp_byte(hsv.v * 255.0);
            }
            report_row(progress);
        }
    });
    ImageBuffer equalized = equalize_channel(value_plane, progress);
    ImageBuffer out(w, h, ch);
    parallel_rows(h, max_threads(), [&](int y0, int y1) {
        for (int y = y0; y < y1; ++y) {
            const unsigned char* src = image.row(y);
            const unsigned char* vsrc = equalized.row(y);
            unsigned char* dst = out.row(y);
            for (int x = 0; x < w; ++x) {
                HsvColor hsv = rgb_to_hsv(src[x * ch], src[x * ch + 1], src[x * ch + 2]);
                hsv.v = vsrc[x] / 255.0;
                unsigned char r;
                unsigned char g;
                unsigned char b;
                hsv_to_rgb(hsv, r, g, b);
                dst[x * ch] = r;
                dst[x * ch + 1] = g;
                dst[x * ch + 2] = b;
            }
            report_row(progress);
        }
    });
    return out;
}

ImageBuffer pixel_add_constant(const ImageBuffer& image, int value, Progress* progress) {
    if (image.empty()) return clone_shape(image);
    int h = image.height();
    int w = image.width();
    int ch = image.channels();
    ImageBuffer out(w, h, ch);
    if (progress) progress->reset(h);
    parallel_rows(h, max_threads(), [&](int y0, int y1) {
        for (int y = y0; y < y1; ++y) {
            const unsigned char* src = image.row(y);
            unsigned char* dst = out.row(y);
            for (int i = 0; i < w * ch; ++i) {
                int v = src[i] + value;
                dst[i] = static_cast<unsigned char>(ImageBuffer::clamp_int(v, 0, 255));
            }
            report_row(progress);
        }
    });
    return out;
}

ImageBuffer pixel_subtract_constant(const ImageBuffer& image, int value, Progress* progress) {
    return pixel_add_constant(image, -value, progress);
}

ImageBuffer pixel_multiply_constant(const ImageBuffer& image, double factor, Progress* progress) {
    if (image.empty()) return clone_shape(image);
    int h = image.height();
    int w = image.width();
    int ch = image.channels();
    ImageBuffer out(w, h, ch);
    if (progress) progress->reset(h);
    parallel_rows(h, max_threads(), [&](int y0, int y1) {
        for (int y = y0; y < y1; ++y) {
            const unsigned char* src = image.row(y);
            unsigned char* dst = out.row(y);
            for (int i = 0; i < w * ch; ++i) {
                dst[i] = clamp_byte(src[i] * factor);
            }
            report_row(progress);
        }
    });
    return out;
}

ImageBuffer pixel_divide_constant(const ImageBuffer& image, double divisor, Progress* progress) {
    if (divisor == 0.0) divisor = 1.0;
    return pixel_multiply_constant(image, 1.0 / divisor, progress);
}

ImageBuffer pixel_negative(const ImageBuffer& image, Progress* progress) {
    if (image.empty()) return clone_shape(image);
    int h = image.height();
    int w = image.width();
    int ch = image.channels();
    ImageBuffer out(w, h, ch);
    if (progress) progress->reset(h);
    parallel_rows(h, max_threads(), [&](int y0, int y1) {
        for (int y = y0; y < y1; ++y) {
            const unsigned char* src = image.row(y);
            unsigned char* dst = out.row(y);
            for (int i = 0; i < w * ch; ++i) {
                dst[i] = static_cast<unsigned char>(255 - src[i]);
            }
            report_row(progress);
        }
    });
    return out;
}

ImageBuffer pixel_gamma(const ImageBuffer& image, double gamma, Progress* progress) {
    if (image.empty()) return clone_shape(image);
    if (gamma <= 0.0) gamma = 1.0;
    int h = image.height();
    int w = image.width();
    int ch = image.channels();
    ImageBuffer out(w, h, ch);
    if (progress) progress->reset(h);
    unsigned char lut[256];
    for (int i = 0; i < 256; ++i) {
        double normalized = i / 255.0;
        lut[i] = clamp_byte(255.0 * pow(normalized, gamma));
    }
    parallel_rows(h, max_threads(), [&](int y0, int y1) {
        for (int y = y0; y < y1; ++y) {
            const unsigned char* src = image.row(y);
            unsigned char* dst = out.row(y);
            for (int i = 0; i < w * ch; ++i) {
                dst[i] = lut[src[i]];
            }
            report_row(progress);
        }
    });
    return out;
}

ImageBuffer pixel_and_constant(const ImageBuffer& image, int value, Progress* progress) {
    if (image.empty()) return clone_shape(image);
    int h = image.height();
    int w = image.width();
    int ch = image.channels();
    ImageBuffer out(w, h, ch);
    if (progress) progress->reset(h);
    int mask = ImageBuffer::clamp_int(value, 0, 255);
    parallel_rows(h, max_threads(), [&](int y0, int y1) {
        for (int y = y0; y < y1; ++y) {
            const unsigned char* src = image.row(y);
            unsigned char* dst = out.row(y);
            for (int i = 0; i < w * ch; ++i) {
                dst[i] = static_cast<unsigned char>(src[i] & mask);
            }
            report_row(progress);
        }
    });
    return out;
}

ImageBuffer pixel_or_constant(const ImageBuffer& image, int value, Progress* progress) {
    if (image.empty()) return clone_shape(image);
    int h = image.height();
    int w = image.width();
    int ch = image.channels();
    ImageBuffer out(w, h, ch);
    if (progress) progress->reset(h);
    int mask = ImageBuffer::clamp_int(value, 0, 255);
    parallel_rows(h, max_threads(), [&](int y0, int y1) {
        for (int y = y0; y < y1; ++y) {
            const unsigned char* src = image.row(y);
            unsigned char* dst = out.row(y);
            for (int i = 0; i < w * ch; ++i) {
                dst[i] = static_cast<unsigned char>(src[i] | mask);
            }
            report_row(progress);
        }
    });
    return out;
}

ImageBuffer pixel_xor_constant(const ImageBuffer& image, int value, Progress* progress) {
    if (image.empty()) return clone_shape(image);
    int h = image.height();
    int w = image.width();
    int ch = image.channels();
    ImageBuffer out(w, h, ch);
    if (progress) progress->reset(h);
    int mask = ImageBuffer::clamp_int(value, 0, 255);
    parallel_rows(h, max_threads(), [&](int y0, int y1) {
        for (int y = y0; y < y1; ++y) {
            const unsigned char* src = image.row(y);
            unsigned char* dst = out.row(y);
            for (int i = 0; i < w * ch; ++i) {
                dst[i] = static_cast<unsigned char>(src[i] ^ mask);
            }
            report_row(progress);
        }
    });
    return out;
}
