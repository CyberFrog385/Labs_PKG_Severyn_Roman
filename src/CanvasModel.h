#ifndef CANVASMODEL_H
#define CANVASMODEL_H

#include <QImage>
#include <QColor>
#include <QSize>
#include <QString>

class CanvasModel {
public:
    explicit CanvasModel(const QSize &size = QSize(800, 600));

    void resize(const QSize &size);
    void clear(const QColor &fillColor = Qt::white);

    bool saveToFile(const QString &fileName, const char *format = nullptr) const;

    QImage& getImage() { return image; }
    const QImage& getImage() const { return image; }

private:
    QImage image;
};

#endif