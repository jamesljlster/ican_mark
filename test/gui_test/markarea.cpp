#include "markarea.h"

#include <cstdio>
#include <iostream>

#include <QBrush>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QRectF>
#include <Qt>

using namespace std;
using namespace ical_mark;

MarkArea::MarkArea(QWidget* parent) : QWidget(parent)
{
    this->setMouseTracking(true);
    this->setFocus();

    this->bgImage = QImage("color_map.png");
    this->setCursor(Qt::BlankCursor);
}

bool MarkArea::event(QEvent* event)
{
    bool ret = false;
    QEvent::Type eventType = event->type();

    if (eventType == QEvent::MouseMove)
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        this->mousePos = me->pos();
        this->testAction.run(me);
        ret = true;
    }
    else if (eventType == QEvent::MouseButtonPress ||
             eventType == QEvent::MouseButtonRelease)
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::MouseButton::LeftButton)
        {
            this->testAction.run(me);
            ret = true;
        }
    }
    else if (eventType == QEvent::KeyPress)
    {
        QKeyEvent* ke = static_cast<QKeyEvent*>(event);
        if (ke->key() == Qt::Key_Escape)
        {
            this->testAction.revert();
            ret = true;
        }
        else if (ke->key() == Qt::Key_Backspace)
        {
            this->testAction.reset();
            ret = true;
        }
    }

    cout << "state: " << this->testAction.state();
    switch (static_cast<RBoxMark::State>(this->testAction.state()))
    {
        case RBoxMark::State::INIT:
            cout << "-" << this->testAction["degree"].state();
            break;

        case RBoxMark::State::DEGREE_FIN:
            cout << "-" << this->testAction["bbox"].state();
            break;

        case RBoxMark::State::BBOX_FIN:
            break;
    }

    if (this->testAction.finish())
    {
        /*
        printf(" [(%d, %d) (%d, %d)] ", this->testAction["pos1"]["press"].x(),
               this->testAction["pos1"]["press"].y(),
               this->testAction["pos1"]["release"].x(),
               this->testAction["pos1"]["release"].y());
        printf(" [(%d, %d) (%d, %d)] ", this->testAction["pos2"]["press"].x(),
               this->testAction["pos2"]["press"].y(),
               this->testAction["pos2"]["release"].x(),
               this->testAction["pos2"]["release"].y());
       */
        cout << " Finished";
    }
    cout << endl;

    if (ret)
    {
        this->repaint();
        return ret;
    }
    else
    {
        return QWidget::event(event);
    }
}

void MarkArea::paintEvent(QPaintEvent* paintEvent)
{
    int width = this->width();
    int height = this->height();

    QPoint center = this->mousePos;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (this->bgImage.isNull())
    {
        painter.setBrush(QBrush(QColor(128, 128, 128), Qt::SolidPattern));
        painter.drawRect(0, 0, width, height);
    }
    else
    {
        QRectF target(0, 0, width, height);
        QRectF source(0, 0, this->bgImage.width(), this->bgImage.height());
        painter.drawImage(target, this->bgImage, source);
    }

    painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    painter.setPen(QColor(160, 160, 160));

    painter.drawLine(QPoint(center.x(), 0), QPoint(center.x(), height));
    painter.drawLine(QPoint(0, center.y()), QPoint(width, center.y()));
}
