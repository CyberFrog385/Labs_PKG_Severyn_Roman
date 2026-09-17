#include "CanvasView.h"
#include <QPainter>

CanvasView::CanvasView(CanvasModel *model, EditorController *controller, QWidget *parent)
    : QWidget(parent), model(model), controller(controller)
{
    setAttribute(Qt::WA_StaticContents);

    connect(controller, &EditorController::canvasUpdated, this, QOverload<>::of(&CanvasView::update));
}

void CanvasView::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    QRect dirtyRect = event->rect();

    painter.drawImage(dirtyRect, model->getImage(), dirtyRect);
}

void CanvasView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        controller->beginStroke(event->pos());
    }
}

void CanvasView::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        controller->continueStroke(event->pos());
    }
}

void CanvasView::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        controller->endStroke(event->pos());
    }
}