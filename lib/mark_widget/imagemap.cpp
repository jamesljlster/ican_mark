#include "mark_widget.h"

#include <QPainterPath>

#include <iostream>

using namespace std;
using namespace ican_mark;

ImageMap::ImageMap(QWidget* parent) : ImageView(parent)
{
    this->setMouseTracking(true);
}

void ImageMap::reset(const QImage& image)
{
    // Call parent reset function
    ImageView::reset(image);

    this->repaint();
}

void ImageMap::set_select_region(const QRectF& selectRegion)
{
    if (this->selectRegion != selectRegion)
    {
        bool selCtrChanged =
            (this->selectRegion.center() != selectRegion.center());

        this->selectRegion = selectRegion;
        this->repaint();

        if (selCtrChanged)
            emit this->selectCenterChanged(this->selectRegion.center());
    }
}

bool ImageMap::event(QEvent* event)
{
    bool ret = false;
    bool selCtrChanged = false;

    QEvent::Type eventType = event->type();

    // Run FSM of clicking action
    if (eventType == QEvent::MouseMove)
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        this->clickAction.run(me);
        ret = true;
    }
    else if (eventType == QEvent::MouseButtonPress ||
             eventType == QEvent::MouseButtonRelease)
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::MouseButton::LeftButton)
        {
            this->clickAction.run(me);
            ret = true;
        }
    }

    // Status checking and processing
    switch (static_cast<ClickAction::State>(this->clickAction.state()))
    {
        case ClickAction::State::MOVE:
            break;

        case ClickAction::State::PRESS:
            do
            {
                QPointF curPos =
                    this->mapping_to_image(this->clickAction["move"]);
                QPointF selCenter = this->selectRegion.center();
                if (curPos != selCenter)
                {
                    this->selectRegion.moveCenter(curPos);
                    selCtrChanged = true;
                }
            } while (0);
            break;

        case ClickAction::State::RELEASE:
            this->clickAction.reset();
            break;
    }

    if (ret)
    {
        this->repaint();

        // Raise signals
        if (selCtrChanged)
            emit selectCenterChanged(this->selectRegion.center());

        return ret;
    }
    else
    {
        return QWidget::event(event);
    }
}

void ImageMap::paintEvent(QPaintEvent* paintEvent)
{
    (void)paintEvent;

    // Paint background
    this->draw_background();

    // Draw selected region
    this->draw_select_region();
}

void ImageMap::resizeEvent(QResizeEvent* event) { this->zoom_to_fit(); }

void ImageMap::draw_select_region()
{
    // Setup painter and drawing style
    QPainter painter(this);

    painter.setPen(QPen(QColor(0, 0, 0, 128), 2));
    painter.setBrush(QColor(0, 0, 0, 96));

    // Draw select region
    QPainterPath path;
    path.addRect(QRectF(0, 0, this->width(), this->height()));
    path.addRect(this->mapping_to_view(selectRegion));

    painter.drawPath(path);
}
