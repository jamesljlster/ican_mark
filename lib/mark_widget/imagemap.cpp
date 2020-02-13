#include "mark_widget.h"

#include <iostream>

using namespace std;
using namespace ical_mark;

ImageMap::ImageMap(QWidget* parent) : ImageView(parent)
{
    this->setMouseTracking(true);
    this->bgImage = QImage("color_map.png");

    this->imageRegion = QRectF(QPoint(0, 0), this->bgImage.size());
    this->viewRegion = this->find_view_region(this->bgImage, this->size());

    this->ratio = 2.0;
    this->selRegion =
        this->find_select_region(this->imageRegion.center(), this->ratio);
}

void ImageMap::reset(const QImage& image) { this->bgImage = image; }

void ImageMap::set_ratio(qreal ratio)
{
    this->ratio = ratio;
    this->selRegion =
        this->find_select_region(this->selRegion.center(), this->ratio);
    this->repaint();
}

bool ImageMap::event(QEvent* event)
{
    bool ret = false;
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
            this->selRegion = this->find_select_region(
                this->mapping_to_image(this->clickAction["move"]), this->ratio);
            break;

        case ClickAction::State::RELEASE:
            this->clickAction.reset();
            break;
    }

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

void ImageMap::paintEvent(QPaintEvent* paintEvent)
{
    int width = this->width();
    int height = this->height();

    (void)paintEvent;

    // Paint background
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setBrush(QBrush(QColor(0, 0, 0), Qt::SolidPattern));
    painter.drawRect(0, 0, width, height);

    if (!this->bgImage.isNull())
    {
        painter.drawImage(this->viewRegion, this->bgImage, this->imageRegion);
    }

    // Draw selected region
    this->draw_select_region(this->mapping_to_view(this->selRegion));
}

void ImageMap::resizeEvent(QResizeEvent* event)
{
    this->viewRegion = this->find_view_region(this->bgImage, event->size());
}

QRectF ImageMap::find_select_region(const QPointF& point, qreal ratio)
{
    qreal imgWidth = this->bgImage.width();
    qreal imgHeight = this->bgImage.height();

    // Find region size
    qreal halfWidth = this->bgImage.width() / (ratio * 2);
    qreal halfHeight = this->bgImage.height() / (ratio * 2);

    // Limit point inside available region
    qreal x = point.x();
    qreal y = point.y();

    if (x < halfWidth)
    {
        x = halfWidth;
    }
    else if (x >= imgWidth - halfWidth)
    {
        x = imgWidth - halfWidth - 1;
    }

    if (y < halfHeight)
    {
        y = halfHeight;
    }
    else if (y >= imgHeight - halfHeight)
    {
        y = imgHeight - halfHeight - 1;
    }

    return QRectF(QPointF(x - halfWidth, y - halfHeight),
                  QPointF(x + halfWidth, y + halfHeight));
}

void ImageMap::draw_select_region(const QRectF& selRegion)
{
    // Setup painter and drawing style
    QPainter painter(this);

    painter.setRenderHint(QPainter::RenderHint::Antialiasing);
    painter.setPen(QPen(QColor(0, 0, 0, 196), 2));

    // Draw anchor point
    painter.drawRect(selRegion);
}
