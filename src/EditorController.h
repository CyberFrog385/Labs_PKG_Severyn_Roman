#ifndef EDITORCONTROLLER_H
#define EDITORCONTROLLER_H

#include <QObject>
#include <QPoint>
#include <QColor>
#include "CanvasModel.h"

enum class ToolType {
    Pen,
    Eraser
};

class EditorController : public QObject {
    Q_OBJECT

public:
    explicit EditorController(CanvasModel *model, QObject *parent = nullptr);

    void setPrimaryColor(const QColor &color) { currentColor = color; }
    void setPenWidth(int width) { penWidth = width; }
    void setCurrentTool(ToolType tool) { activeTool = tool; }

    void beginStroke(const QPoint &pos);
    void continueStroke(const QPoint &pos);
    void endStroke(const QPoint &pos);

signals:
    void canvasUpdated();

private:
    CanvasModel *model;
    QPoint lastPoint;
    QColor currentColor{Qt::black};
    int penWidth{5};
    ToolType activeTool{ToolType::Pen};
    bool isDrawing{false};

    void drawLineTo(const QPoint &endPoint);
};

#endif