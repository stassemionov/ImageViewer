#ifndef LABELVIEWER_H
#define LABELVIEWER_H

#include <QLabel>
#include <QEvent>
#include <QMouseEvent>

class LabelViewer : public QLabel
{
    Q_OBJECT

public:
    explicit LabelViewer(QWidget *parent = nullptr);
    ~LabelViewer();

signals:
    void mouseEnterSignal();
    void mouseLeaveSignal();
    void mouseMoveSignal(int, int);
    void mousePressSignal(Qt::MouseButton, QPoint);
    void imageMoveSignal(QPoint pos);
    void imageScaledSignal(bool direction);

protected:
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
    void contextMenuEvent(QContextMenuEvent* event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent* event);

private:
    QPoint m_drag_current_pos;
};

#endif // LABELVIEWER_H
