#include "core/clahe.h"

#include <cstddef>
#include <cmath>
#include <vector>

#include "core/color.h"
#include "core/processing.h"

namespace {

void report_row(Progress* progress) {
    if (progress) {
        progress->value.fetch_add(1);
    }
}

std::vector<unsigned char> build_tile_maps(const ImageBuffer& plane,
                                           int tiles_x,
                                           int tiles_y,
                                           double clip_factor) {
    int w = plane.width();
    int h = plane.height();
    int tile_w = (w + tiles_x - 1) / tiles_x;
    int tile_h = (h + tiles_y - 1) / tiles_y;
    if (tile_w < 1) tile_w = 1;
    if (tile_h < 1) tile_h = 1;
    std::vector<unsigned char> maps(static_cast<size_t>(tiles_x) * tiles_y * 256, 0);

    for (int ty = 0; ty < tiles_y; ++ty) {
        for (int tx = 0; tx < tiles_x; ++tx) {
            int x0 = tx * tile_w;
            int y0 = ty * tile_h;
            int x1 = x0 + tile_w;
            int y1 = y0 + tile_h;
            if (x1 > w) x1 = w;
            if (y1 > h) y1 = h;
            int pixels = (x1 - x0) * (y1 - y0);
            if (pixels < 1) pixels = 1;

            int histogram[256];
            for (int i = 0; i < 256; ++i) histogram[i] = 0;
            for (int y = y0; y < y1; ++y) {
                const unsigned char* row = plane.row(y);
                for (int x = x0; x < x1; ++x) {
                    histogram[row[x]] += 1;
                }
            }

            double limit = clip_factor * pixels / 256.0;
            if (limit < 1.0) limit = 1.0;
            int limit_int = static_cast<int>(limit);
            int clipped = 0;
            for (int i = 0; i < 256; ++i) {
                if (histogram[i] > limit_int) {
                    clipped += histogram[i] - limit_int;
                    histogram[i] = limit_int;
                }
            }
            int to_redistribute = clipped;
            int step = 0;
            while (to_redistribute > 0) {
                if (histogram[step] < limit_int) {
                    histogram[step] += 1;
                    to_redistribute -= 1;
                }
                step += 1;
                if (step >= 256) step = 0;
            }

            unsigned char* map = &maps[(static_cast<size_t>(ty) * tiles_x + tx) * 256];
            int cdf = 0;
            for (int i = 0; i < 256; ++i) {
                cdf += histogram[i];
                int value = cdf * 255 / pixels;
                if (value < 0) value = 0;
                if (value > 255) value = 255;
                map[i] = static_cast<unsigned char>(value);
            }
        }
    }
    return maps;
}

ImageBuffer clahe_plane(const ImageBuffer& plane,
                        const std::vector<unsigned char>& maps,
                        int tiles_x,
                        int tiles_y,
                        Progress* progress) {
    int w = plane.width();
    int h = plane.height();
    ImageBuffer out(w, h, 1);
    int tile_w = (w + tiles_x - 1) / tiles_x;
    int tile_h = (h + tiles_y - 1) / tiles_y;
    if (tile_w < 1) tile_w = 1;
    if (tile_h < 1) tile_h = 1;
    parallel_rows(h, max_threads(), [&](int y0, int y1) {
        for (int y = y0; y < y1; ++y) {
            const unsigned char* src = plane.row(y);
            unsigned char* dst = out.row(y);
            double gy = (y + 0.5) / tile_h - 0.5;
            int ty0 = static_cast<int>(floor(gy));
            double fy = gy - ty0;
            int ty1 = ty0 + 1;
            ty0 = ImageBuffer::clamp_int(ty0, 0, tiles_y - 1);
            ty1 = ImageBuffer::clamp_int(ty1, 0, tiles_y - 1);
            for (int x = 0; x < w; ++x) {
                double gx = (x + 0.5) / tile_w - 0.5;
                int tx0 = static_cast<int>(floor(gx));
                double fx = gx - tx0;
                int tx1 = tx0 + 1;
                tx0 = ImageBuffer::clamp_int(tx0, 0, tiles_x - 1);
                tx1 = ImageBuffer::clamp_int(tx1, 0, tiles_x - 1);
                int v = src[x];
                double v00 = maps[(static_cast<size_t>(ty0) * tiles_x + tx0) * 256 + v];
                double v10 = maps[(static_cast<size_t>(ty0) * tiles_x + tx1) * 256 + v];
                double v01 = maps[(static_cast<size_t>(ty1) * tiles_x + tx0) * 256 + v];
                double v11 = maps[(static_cast<size_t>(ty1) * tiles_x + tx1) * 256 + v];
                double top = v00 * (1.0 - fx) + v10 * fx;
                double bottom = v01 * (1.0 - fx) + v11 * fx;
                double value = top * (1.0 - fy) + bottom * fy;
                dst[x] = static_cast<unsigned char>(value + 0.5);
            }
            report_row(progress);
        }
    });
    return out;
}

}

ImageBuffer clahe(const ImageBuffer& image, int tiles_per_axis, double clip_factor, Progress* progress) {
    if (image.empty()) {
        return ImageBuffer(image.width(), image.height(), image.channels());
    }
    if (tiles_per_axis < 2) tiles_per_axis = 2;
    if (tiles_per_axis > 32) tiles_per_axis = 32;
    if (clip_factor < 0.5) clip_factor = 0.5;

    int w = image.width();
    int h = image.height();
    int ch = image.channels();

    if (ch == 1) {
        if (progress) progress->reset(h);
        std::vector<unsigned char> maps = build_tile_maps(image, tiles_per_axis, tiles_per_axis, clip_factor);
        return clahe_plane(image, maps, tiles_per_axis, tiles_per_axis, progress);
    }

    if (progress) progress->reset(h * 3);
    ImageBuffer value_plane(w, h, 1);
    parallel_rows(h, max_threads(), [&](int y0, int y1) {
        for (int y = y0; y < y1; ++y) {
            const unsigned char* src = image.row(y);
            unsigned char* dst = value_plane.row(y);
            for (int x = 0; x < w; ++x) {
                HsvColor hsv = rgb_to_hsv(src[x * ch], src[x * ch + 1], src[x * ch + 2]);
                dst[x] = static_cast<unsigned char>(hsv.v * 255.0 + 0.5);
            }
            report_row(progress);
        }
    });

    std::vector<unsigned char> maps = build_tile_maps(value_plane, tiles_per_axis, tiles_per_axis, clip_factor);
    ImageBuffer equalized = clahe_plane(value_plane, maps, tiles_per_axis, tiles_per_axis, progress);

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
