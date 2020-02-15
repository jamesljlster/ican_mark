#include "mark_widget.h"

using namespace std;
using namespace ical_mark;

ImageView::ImageView(QWidget* parent) : QWidget(parent) {}

void ImageView::reset(const QImage& image) { this->bgImage = image; }

QRectF ImageView::find_image_region(const QPointF& center, const QSizeF& size)
{
    qreal imgWidth = this->bgImage.width();
    qreal imgHeight = this->bgImage.height();

    // Limit region size inside available region
    qreal halfWidth = size.width();
    qreal halfHeight = size.height();

    if (halfWidth > this->bgImage.width())
    {
        halfWidth = this->bgImage.width();
    }

    if (halfHeight > this->bgImage.height())
    {
        halfHeight = this->bgImage.height();
    }

    halfWidth /= 2.0;
    halfHeight /= 2.0;

    // Limit point inside available region
    qreal x = center.x();
    qreal y = center.y();

    if (x < halfWidth)
    {
        x = halfWidth;
    }
    else if (x >= imgWidth - halfWidth)
    {
        x = imgWidth - halfWidth;
    }

    if (y < halfHeight)
    {
        y = halfHeight;
    }
    else if (y >= imgHeight - halfHeight)
    {
        y = imgHeight - halfHeight;
    }

    return QRectF(QPointF(x - halfWidth, y - halfHeight),
                  QPointF(x + halfWidth, y + halfHeight));
}

QRectF ImageView::find_view_region(const QSize& sizeHint,
                                   const QSize& widgetSize)
{
    QSize markSize = sizeHint.scaled(widgetSize, Qt::KeepAspectRatio);
    QPoint markBase = QPoint((widgetSize.width() - markSize.width()) / 2,
                             (widgetSize.height() - markSize.height()) / 2);
    return QRectF(markBase, markSize);
}

QPointF ImageView::scaling_to_view(const QPointF& point)
{
    return QPointF(
        point.x() * this->viewRegion.width() / this->imageRegion.width(),
        point.y() * this->viewRegion.height() / this->imageRegion.height());
}

QSizeF ImageView::scaling_to_view(const QSizeF& size)
{
    QPointF tmp(size.width(), size.height());
    tmp = this->scaling_to_view(tmp);
    return QSizeF(tmp.x(), tmp.y());
}

QPointF ImageView::mapping_to_view(const QPointF& point)
{
    return this->scaling_to_view(point - this->imageRegion.topLeft()) +
           this->viewRegion.topLeft();
}

QRectF ImageView::mapping_to_view(const QRectF& rect)
{
    QPointF point = this->mapping_to_view(rect.topLeft());
    QSizeF size = this->scaling_to_view(rect.size());

    return QRectF(point, size);
}

QPointF ImageView::scaling_to_image(const QPointF& point)
{
    return QPointF(
        point.x() * this->imageRegion.width() / this->viewRegion.width(),
        point.y() * this->imageRegion.height() / this->viewRegion.height());
}

QSizeF ImageView::scaling_to_image(const QSizeF& size)
{
    QPointF tmp(size.width(), size.height());
    tmp = this->scaling_to_image(tmp);
    return QSizeF(tmp.x(), tmp.y());
}

QPointF ImageView::mapping_to_image(const QPointF& point)
{
    return this->scaling_to_image(point - this->viewRegion.topLeft()) +
           this->imageRegion.topLeft();
}

QRectF ImageView::mapping_to_image(const QRectF& rect)
{
    QPointF point = this->mapping_to_image(rect.topLeft());
    QSizeF size = this->scaling_to_image(rect.size());

    return QRectF(point, size);
}
