#include "CanvasModel.h"

CanvasModel::CanvasModel(const QSize &size) {
    resize(size);
    clear();
}

void CanvasModel::resize(const QSize &size) {
    if (image.size() == size) return;
    QImage newImage(size, QImage::Format_ARGB32);
    newImage.fill(Qt::white);
    image = newImage;
}

void CanvasModel::clear(const QColor &fillColor) {
    image.fill(fillColor);
}

bool CanvasModel::saveToFile(const QString &fileName, const char *format) const {
    if (fileName.isEmpty()) return false;
    return image.save(fileName, format);
}