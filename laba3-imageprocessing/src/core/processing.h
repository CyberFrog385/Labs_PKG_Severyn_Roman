#ifndef PROCESSING_H
#define PROCESSING_H

#include "core/image_buffer.h"
#include "core/parallel.h"

void compute_histogram(const ImageBuffer& image, int channel, int histogram[256]);
void compute_statistics(const ImageBuffer& image, int channel, double out[4]);

ImageBuffer linear_contrast_auto(const ImageBuffer& image, Progress* progress);
ImageBuffer linear_contrast_manual(const ImageBuffer& image, int in_min, int in_max, Progress* progress);

ImageBuffer equalize_channel(const ImageBuffer& image, Progress* progress);
ImageBuffer equalize_rgb(const ImageBuffer& image, Progress* progress);
ImageBuffer equalize_hsv(const ImageBuffer& image, Progress* progress);

ImageBuffer pixel_add_constant(const ImageBuffer& image, int value, Progress* progress);
ImageBuffer pixel_subtract_constant(const ImageBuffer& image, int value, Progress* progress);
ImageBuffer pixel_multiply_constant(const ImageBuffer& image, double factor, Progress* progress);
ImageBuffer pixel_divide_constant(const ImageBuffer& image, double divisor, Progress* progress);
ImageBuffer pixel_negative(const ImageBuffer& image, Progress* progress);
ImageBuffer pixel_gamma(const ImageBuffer& image, double gamma, Progress* progress);
ImageBuffer pixel_and_constant(const ImageBuffer& image, int value, Progress* progress);
ImageBuffer pixel_or_constant(const ImageBuffer& image, int value, Progress* progress);
ImageBuffer pixel_xor_constant(const ImageBuffer& image, int value, Progress* progress);

#endif
