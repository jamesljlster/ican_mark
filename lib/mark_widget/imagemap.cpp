#include "mark_widget.h"

#include <QPainterPath>

#include <iostream>

using namespace std;
using namespace ical_mark;

ImageMap::ImageMap(QWidget* parent) : ImageView(parent)
{
    this->setMouseTracking(true);
}

void ImageMap::reset(const QImage& image)
{
    // Call parent reset function
    ImageView::reset(image);

    // Set regions
    this->imageRegion = QRectF(QPoint(0, 0), this->bgImage.size());
    this->viewRegion =
        this->find_view_region(this->imageRegion.size().toSize(), this->size());

    this->repaint();
}

void ImageMap::set_select_region(const QRectF& selectRegion)
{
    if (this->selectRegion != selectRegion)
    {
        this->selectRegion = selectRegion;
        this->repaint();
        emit this->selectRegionChanged(this->selectRegion);
    }
}

const QRectF ImageMap::get_select_region() { return this->selectRegion; }

bool ImageMap::event(QEvent* event)
{
    bool ret = false;
    bool selRegionChanged = false;

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
                QRectF newSelectRegion = this->find_image_region(
                    this->mapping_to_image(this->clickAction["move"]),
                    this->selectRegion.size());
                if (this->selectRegion != newSelectRegion)
                {
                    this->selectRegion = newSelectRegion;
                    selRegionChanged = true;
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
        if (selRegionChanged) emit selectRegionChanged(this->selectRegion);

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
    this->draw_select_region(this->mapping_to_view(this->selectRegion),
                             this->viewRegion);
}

void ImageMap::resizeEvent(QResizeEvent* event)
{
    this->viewRegion = this->find_view_region(this->imageRegion.size().toSize(),
                                              event->size());
}

void ImageMap::draw_select_region(const QRectF& selRegion,
                                  const QRectF& viewRegion)
{
    // Setup painter and drawing style
    QPainter painter(this);

    painter.setPen(QPen(QColor(0, 0, 0, 128), 2));
    painter.setBrush(QColor(0, 0, 0, 96));

    // Draw select region
    QPainterPath path;
    path.addRect(viewRegion);
    path.addRect(selRegion);

    painter.drawPath(path);
}
