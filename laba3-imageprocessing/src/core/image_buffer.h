#ifndef IMAGE_BUFFER_H
#define IMAGE_BUFFER_H

#include <vector>

class ImageBuffer {
public:
    ImageBuffer();
    ImageBuffer(int width, int height, int channels);

    bool empty() const;
    int width() const;
    int height() const;
    int channels() const;
    int row_stride() const;

    unsigned char* row(int y);
    const unsigned char* row(int y) const;

    unsigned char get(int x, int y, int channel) const;
    void set(int x, int y, int channel, unsigned char value);

    static int clamp_int(int value, int low, int high);
    static int clamp_coord(int value, int size);
    static int reflect_coord(int value, int size);

private:
    int w;
    int h;
    int ch;
    int stride;
    std::vector<unsigned char> data;
};

#endif
