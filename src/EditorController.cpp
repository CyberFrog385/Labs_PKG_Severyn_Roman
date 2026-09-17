#include "EditorController.h"
#include <QPainter>

EditorController::EditorController(CanvasModel *model, QObject *parent)
    : QObject(parent), model(model) {}

void EditorController::beginStroke(const QPoint &pos) {
    isDrawing = true;
    lastPoint = pos;
    drawLineTo(pos);
}

void EditorController::continueStroke(const QPoint &pos) {
    if (!isDrawing) return;
    drawLineTo(pos);
    lastPoint = pos;
}

void EditorController::endStroke(const QPoint &pos) {
    if (!isDrawing) return;
    drawLineTo(pos);
    isDrawing = false;
}

void EditorController::drawLineTo(const QPoint &endPoint) {
    QPainter painter(&model->getImage());
    painter.setRenderHint(QPainter::Antialiasing, true);

    QColor drawColor = (activeTool == ToolType::Eraser) ? Qt::white : currentColor;
    painter.setPen(QPen(drawColor, penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawLine(lastPoint, endPoint);

    emit canvasUpdated();
}