#ifndef GRADIENTSLIDER_H
#define GRADIENTSLIDER_H

#include <QSlider>
#include <QColor>
#include <QList>

class GradientSlider : public QSlider {
    Q_OBJECT

public:
    explicit GradientSlider(Qt::Orientation orientation = Qt::Horizontal, QWidget *parent = nullptr);

    void setGradient(const QColor &left, const QColor &right);
    void setGradientStops(const QList<QColor> &stops);
    void clearGradient() { hasGradient = false; hasStops = false; update(); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QColor left = Qt::white;
    QColor right = Qt::black;
    QList<QColor> stops;
    bool hasGradient = false;
    bool hasStops = false;
};

#endif