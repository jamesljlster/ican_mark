#include "markarea.h"

#include <cmath>
#include <cstdio>
#include <iostream>

#include <QBrush>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QRectF>
#include <Qt>

#define __M_PI 3.14159265359

using namespace std;
using namespace ical_mark;

vector<QLine> draw_aim_crosshair(QPaintDevice* widget, const QPoint& center,
                                 float degree, const QColor& penColor)
{
    int width = widget->width();
    int height = widget->height();

    // Setup painter and drawing style
    QPainter painter(widget);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    painter.setPen(penColor);

    // Find lines of crosshair
    float shift = fmod(degree + 225, 90) - 45;
    shift = tan(shift * __M_PI / 180.0);

    QLine line1(  //
        QPoint(center.x() - shift * center.y(), 0),
        QPoint(center.x() + shift * (height - center.y()), height));
    QLine line2(  //
        QPoint(0, center.y() + shift * center.x()),
        QPoint(width, center.y() - shift * (width - center.x())));

    // Draw aim crosshair
    painter.drawLine(line1);
    painter.drawLine(line2);

    return {line1, line2};
}

MarkArea::MarkArea(QWidget* parent) : QWidget(parent)
{
    this->setMouseTracking(true);
    this->setFocus();

    this->bgImage = QImage("color_map.png");
    // this->setCursor(Qt::BlankCursor);
}

bool MarkArea::event(QEvent* event)
{
    bool ret = false;
    QEvent::Type eventType = event->type();

    // Run FSM of marking action
    if (eventType == QEvent::MouseMove)
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        this->mousePos = me->pos();
        this->markAction.run(me);
        ret = true;
    }
    else if (eventType == QEvent::MouseButtonPress ||
             eventType == QEvent::MouseButtonRelease)
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::MouseButton::LeftButton)
        {
            this->markAction.run(me);
            ret = true;
        }
    }
    else if (eventType == QEvent::KeyPress)
    {
        QKeyEvent* ke = static_cast<QKeyEvent*>(event);
        if (ke->key() == Qt::Key_Escape)
        {
            this->markAction.revert();
            ret = true;
        }
        else if (ke->key() == Qt::Key_Backspace)
        {
            this->markAction.reset();
            ret = true;
        }
    }

    // Status checking and processing
    cout << "state: " << this->markAction.state();
    switch (static_cast<RBoxMark::State>(this->markAction.state()))
    {
        case RBoxMark::State::INIT:

            if (static_cast<TwiceClick::State>(
                    this->markAction["degree"].state()) ==
                TwiceClick::State::POS1_FIN)
            {
                // Calculate and set degree
                this->degree = this->find_degree(
                    this->markAction["degree"]["pos1"]["release"],
                    this->markAction["degree"]["pos2"]["move"]);
            }
            else
            {
                // Reset degree
                this->degree = 0;
            }

            cout << "-" << this->markAction["degree"].state();
            break;

        case RBoxMark::State::DEGREE_FIN:

            // Calculate and set degree
            this->degree = this->find_degree(
                this->markAction["degree"]["pos1"]["release"],
                this->markAction["degree"]["pos2"]["release"]);

            cout << "-" << this->markAction["bbox"].state();
            break;

        case RBoxMark::State::BBOX_FIN:
            break;
    }

    cout << " degree: " << this->degree;

    if (this->markAction.finish())
    {
        /*
        printf(" [(%d, %d) (%d, %d)] ", this->markAction["pos1"]["press"].x(),
               this->markAction["pos1"]["press"].y(),
               this->markAction["pos1"]["release"].x(),
               this->markAction["pos1"]["release"].y());
        printf(" [(%d, %d) (%d, %d)] ", this->markAction["pos2"]["press"].x(),
               this->markAction["pos2"]["press"].y(),
               this->markAction["pos2"]["release"].x(),
               this->markAction["pos2"]["release"].y());
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

    // Paint background
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (this->bgImage.isNull())
    {
        painter.setBrush(QBrush(QColor(128, 128, 128), Qt::SolidPattern));
        painter.drawRect(0, 0, width, height);
    }
    else
    {
        QRectF bgRect(0, 0, width, height);
        QRectF srcRect(0, 0, this->bgImage.width(), this->bgImage.height());
        painter.drawImage(bgRect, this->bgImage, srcRect);
    }

    // Draw aim crosshair
    draw_aim_crosshair(this, this->mousePos, this->degree);

    // Draw marking
    switch (static_cast<RBoxMark::State>(this->markAction.state()))
    {
        case RBoxMark::State::INIT:
            switch (static_cast<TwiceClick::State>(
                this->markAction["degree"].state()))
            {
                case TwiceClick::State::INIT:
                    break;

                case TwiceClick::State::POS1_FIN:
                    break;

                case TwiceClick::State::POS2_FIN:
                    break;
            }

            break;

        case RBoxMark::State::DEGREE_FIN:
            break;

        case RBoxMark::State::BBOX_FIN:
            break;
    }
}

float MarkArea::find_degree(const QPoint& from, const QPoint& to)
{
    double theta = atan2(from.y() - to.y(), to.x() - from.x());
    theta = theta * 180.0 / __M_PI - 90.0;
    if (theta < -180.0)
    {
        theta += 360.0;
    }

    return theta;
}
