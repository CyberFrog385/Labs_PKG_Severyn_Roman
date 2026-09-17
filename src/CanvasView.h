#ifndef CANVASVIEW_H
#define CANVASVIEW_H

#include <QWidget>
#include <QMouseEvent>
#include <QPaintEvent>
#include "CanvasModel.h"
#include "EditorController.h"

class CanvasView : public QWidget {
    Q_OBJECT

public:
    CanvasView(CanvasModel *model, EditorController *controller, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    CanvasModel *model;
    EditorController *controller;
};

#endif