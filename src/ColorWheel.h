#ifndef COLORWHEEL_H
#define COLORWHEEL_H

#include <QWidget>
#include <QImage>
#include <QPoint>
#include "ColorModel.h"

class ColorWheel : public QWidget {
    Q_OBJECT

public:
    explicit ColorWheel(QWidget *parent = nullptr);

    void setHueSat(double h, double s);
    void setValue(double v);

signals:
    void hueSatChanged(double h, double s);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    double hue = 0.0;
    double sat = 0.0;
    double value = 1.0;
    QImage image;

    double radius() const;
    QPointF center() const;
    void updateImage();
    void updateFromPos(const QPoint &pos);
};

#endif