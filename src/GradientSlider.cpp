#include "GradientSlider.h"

#include <QPainter>
#include <QStyle>
#include <QStyleOptionSlider>
#include <QPaintEvent>
#include <algorithm>

GradientSlider::GradientSlider(Qt::Orientation orientation, QWidget *parent)
    : QSlider(orientation, parent)
{
}

void GradientSlider::setGradient(const QColor &leftColor, const QColor &rightColor)
{
    left = leftColor;
    right = rightColor;
    hasGradient = true;
    update();
}

void GradientSlider::setGradientStops(const QList<QColor> &stopsList)
{
    stops = stopsList;
    hasStops = true;
    update();
}

void GradientSlider::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF groove = this->rect();
    if (orientation() == Qt::Horizontal) {
        groove.setTop(groove.top() + groove.height() / 2.0 - 4.0);
        groove.setHeight(8.0);
    } else {
        groove.setLeft(groove.left() + groove.width() / 2.0 - 4.0);
        groove.setWidth(8.0);
    }

    QLinearGradient gradient;
    if (orientation() == Qt::Horizontal) {
        gradient.setStart(groove.left(), 0.0);
        gradient.setFinalStop(groove.right(), 0.0);
    } else {
        gradient.setStart(0.0, groove.top());
        gradient.setFinalStop(0.0, groove.bottom());
    }

    if (hasStops && stops.size() > 1) {
        gradient.setColorAt(0.0, stops.first());
        for (int i = 1; i < stops.size(); ++i)
            gradient.setColorAt(double(i) / double(stops.size() - 1), stops.at(i));
    } else {
        gradient.setColorAt(0.0, hasGradient ? left : QColor(200, 200, 200));
        gradient.setColorAt(1.0, hasGradient ? right : QColor(100, 100, 100));
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(gradient);
    painter.drawRoundedRect(groove, 2.0, 2.0);

    QStyleOptionSlider opt;
    initStyleOption(&opt);
    const QRect handleRect = style()->subControlRect(
        QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

    int alpha = 170;
    if (!isEnabled())
        alpha = 80;
    else if (isSliderDown())
        alpha = 210;

    painter.setBrush(QColor(170, 170, 170, alpha));
    painter.setPen(QColor(90, 90, 90, 120));
    const QPointF center = handleRect.center();
    const double radius = std::min(handleRect.width(), handleRect.height()) / 2.0;
    painter.drawEllipse(center, radius, radius);
}