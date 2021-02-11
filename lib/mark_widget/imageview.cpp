#include "mark_widget.h"

using namespace std;
using namespace ican_mark;

ImageView::ImageView(QWidget* parent) : QWidget(parent) {}

void ImageView::reset(const QImage& image)
{
    this->bgImage = image;
    this->zoom_to_fit();
}

void ImageView::zoom_to_fit()
{
    this->viewScale = this->find_fit_scale_ratio();
    this->viewCenter = this->find_centered_point();
}

double ImageView::find_fit_scale_ratio() const
{
    QSizeF viewSize =
        QSizeF(this->bgImage.size()).scaled(this->size(), Qt::KeepAspectRatio);
    return (double)viewSize.width() / (double)this->bgImage.width();
}

QPointF ImageView::find_centered_point() const
{
    return QPointF(this->width(), this->height()) / 2.0;
}

QPointF ImageView::scaling_to_view(const QPointF& point)
{
    return this->scaling_to_view<QPointF>(point);
}

QSizeF ImageView::scaling_to_view(const QSizeF& size)
{
    QPointF tmp(size.width(), size.height());
    tmp = this->scaling_to_view(tmp);
    return QSizeF(tmp.x(), tmp.y());
}

QPointF ImageView::mapping_to_view(const QPointF& point)
{
    return this->mapping_to_view<QPointF>(point);
}

QRectF ImageView::mapping_to_view(const QRectF& rect)
{
    QPointF point = this->mapping_to_view(rect.topLeft());
    QSizeF size = this->scaling_to_view(rect.size());

    return QRectF(point, size);
}

QPointF ImageView::scaling_to_image(const QPointF& point)
{
    return this->scaling_to_image<QPointF>(point);
}

QSizeF ImageView::scaling_to_image(const QSizeF& size)
{
    QPointF tmp(size.width(), size.height());
    tmp = this->scaling_to_image(tmp);
    return QSizeF(tmp.x(), tmp.y());
}

QPointF ImageView::mapping_to_image(const QPointF& point)
{
    return this->mapping_to_image<QPointF>(point);
}

QRectF ImageView::mapping_to_image(const QRectF& rect)
{
    QPointF point = this->mapping_to_image(rect.topLeft());
    QSizeF size = this->scaling_to_image(rect.size());

    return QRectF(point, size);
}

void ImageView::draw_background(const QColor& bgColor)
{
    int width = this->width();
    int height = this->height();

    // Setup painter
    QPainter painter(this);

    // Paint solid background
    painter.setBrush(QBrush(bgColor, Qt::SolidPattern));
    painter.drawRect(0, 0, width, height);

    // Paint image
    if (!this->bgImage.isNull())
    {
        if (this->viewScale != this->currentScale)
        {
            this->currentScale = this->viewScale;
            this->scaledImg =
                this->bgImage.scaled(this->bgImage.size() * this->viewScale);
        }

        QPointF drawPoint =
            this->viewCenter - QPointF(this->scaledImg.width() / 2.0,
                                       this->scaledImg.height() / 2.0);

        // Paint image
        painter.drawImage(drawPoint, this->scaledImg,
                          QRect(QPoint(0, 0), this->scaledImg.size()));
    }
}
