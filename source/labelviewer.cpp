#include <labelviewer.h>

#include <QApplication>

LabelViewer::LabelViewer(QWidget *parent)
    : QLabel(parent)
{
    this->setMouseTracking(true);
}

LabelViewer::~LabelViewer()
{

}

void LabelViewer::enterEvent(QEvent* event)
{
    event->accept();
    emit mouseEnterSignal();
}

void LabelViewer::leaveEvent(QEvent* event)
{
    event->accept();
    emit mouseLeaveSignal();
}

void LabelViewer::contextMenuEvent(QContextMenuEvent* event)
{
    event->accept();
//    emit contextMenuSignal();
}

void LabelViewer::mousePressEvent(QMouseEvent *event)
{
    event->accept();

    Qt::MouseButton button = event->button();
    emit mousePressSignal(button, event->globalPos());

    if (event->button() == Qt::LeftButton)
    {
        m_drag_current_pos = event->globalPos();
    }
}

void LabelViewer::mouseMoveEvent(QMouseEvent *event)
{
    if (this->pixmap() != nullptr)
    {
        event->accept();
        int x = event->pos().x();
        int y = this->height() - event->pos().y() - 1;
        emit mouseMoveSignal(x, y);

        if (event->buttons() & Qt::LeftButton)
        {
            QPoint glob_pos = event->globalPos();
            QPoint dif = glob_pos - m_drag_current_pos;
            emit imageMoveSignal(dif);
            m_drag_current_pos = glob_pos;
        }
    }
}

void LabelViewer::wheelEvent(QWheelEvent* event)
{
    if (this->pixmap() != nullptr)
    {
        event->accept();
        emit imageScaledSignal(event->angleDelta().y() > 0);
    }
}
