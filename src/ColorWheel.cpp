#include "ColorWheel.h"

#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QtMath>
#include <cmath>

ColorWheel::ColorWheel(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(220, 220);
    updateImage();
}

double ColorWheel::radius() const
{
    return std::min(width(), height()) / 2.0 - 4.0;
}

QPointF ColorWheel::center() const
{
    return QPointF(width() / 2.0, height() / 2.0);
}

void ColorWheel::setHueSat(double h, double s)
{
    hue = h;
    sat = s;
    update();
}

void ColorWheel::setValue(double v)
{
    value = v;
    updateImage();
}

void ColorWheel::updateImage()
{
    const double r = radius();
    const QPointF c = center();
    image = QImage(width(), height(), QImage::Format_RGB32);
    for (int y = 0; y < height(); ++y) {
        for (int x = 0; x < width(); ++x) {
            const double dx = x - c.x();
            const double dy = y - c.y();
            const double dist = std::sqrt(dx * dx + dy * dy);
            if (dist > r) {
                image.setPixel(x, y, qRgb(40, 40, 40));
                continue;
            }
            const double h = std::fmod(std::atan2(c.y() - y, x - c.x()) * 180.0 / M_PI + 360.0, 360.0);
            const double s = dist / r;
            const RGBColor col = ColorModel::hsvToRgb({h, s, value});
            image.setPixel(x, y, qRgb(int(col.r * 255.0 + 0.5), int(col.g * 255.0 + 0.5), int(col.b * 255.0 + 0.5)));
        }
    }
    update();
}

void ColorWheel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawImage(rect(), image);

    const double r = radius();
    const QPointF c = center();
    const double angle = hue * M_PI / 180.0;
    const QPointF pos(c.x() + std::cos(angle) * sat * r, c.y() - std::sin(angle) * sat * r);

    painter.setPen(QPen(Qt::white, 2.0));
    painter.setBrush(QColor(0, 0, 0, 40));
    painter.drawEllipse(pos, 6.0, 6.0);

    painter.setPen(QPen(QColor(0, 0, 0, 90), 1.0));
    painter.drawEllipse(c, r, r);
}

void ColorWheel::updateFromPos(const QPoint &pos)
{
    const QPointF c = center();
    const double dx = pos.x() - c.x();
    const double dy = c.y() - pos.y();
    const double r = radius();
    sat = std::min(std::sqrt(dx * dx + dy * dy) / r, 1.0);
    hue = std::fmod(std::atan2(dy, dx) * 180.0 / M_PI + 360.0, 360.0);
    emit hueSatChanged(hue, sat);
    update();
}

void ColorWheel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        updateFromPos(event->pos());
}

void ColorWheel::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
        updateFromPos(event->pos());
}