#ifndef CLAHE_H
#define CLAHE_H

#include "core/image_buffer.h"
#include "core/parallel.h"

ImageBuffer clahe(const ImageBuffer& image, int tiles_per_axis, double clip_factor, Progress* progress);

#endif
