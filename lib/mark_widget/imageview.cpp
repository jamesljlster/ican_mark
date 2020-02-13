#include "mark_widget.h"

using namespace std;
using namespace ical_mark;

ImageView::ImageView(QWidget* parent) : QWidget(parent) {}

void ImageView::reset(const QImage& image) { this->bgImage = image; }

QRectF ImageView::find_view_region(const QSize& viewSize,
                                   const QSize& widgetSize)
{
    QSize markSize = viewSize.scaled(widgetSize, Qt::KeepAspectRatio);
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
    return this->scaling_to_view(point) + this->viewRegion.topLeft();
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
    return this->scaling_to_image(point - this->viewRegion.topLeft());
}

QRectF ImageView::mapping_to_image(const QRectF& rect)
{
    QPointF point = this->mapping_to_image(rect.topLeft());
    QSizeF size = this->scaling_to_image(rect.size());

    return QRectF(point, size);
}
