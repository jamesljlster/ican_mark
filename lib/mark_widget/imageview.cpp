#include "mark_widget.h"

using namespace std;
using namespace ican_mark;

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
    return this->scaling_to_image(point - this->viewRegion.topLeft()) +
           this->imageRegion.topLeft();
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
        QRectF imReg = this->imageRegion;

        // Get image size
        int imWidth = this->bgImage.width();
        int imHeight = this->bgImage.height();

        // Limit image region to valid area and find padding size
        QPointF padTopLeft = QPointF(0, 0);
        QPointF topLeft = imReg.topLeft();

        if (topLeft.x() < 0)
        {
            padTopLeft.setX(0 - topLeft.x());
            topLeft.setX(0);
        }

        if (topLeft.y() < 0)
        {
            padTopLeft.setY(0 - topLeft.y());
            topLeft.setY(0);
        }

        QPointF padBtmRight = QPointF(0, 0);
        QPointF btmRight = imReg.bottomRight();

        if (btmRight.x() > imWidth)
        {
            padBtmRight.setX(btmRight.x() - imWidth);
            btmRight.setX(imWidth);
        }

        if (btmRight.y() > imHeight)
        {
            padBtmRight.setY(btmRight.y() - imHeight);
            btmRight.setY(imHeight);
        }

        // Set new image region
        imReg.setTopLeft(topLeft);
        imReg.setBottomRight(btmRight);

        // Paint image with padding
        QRectF drawRegion =
            QRectF(this->viewRegion.topLeft() + padTopLeft,
                   this->viewRegion.bottomRight() - padBtmRight);
        painter.drawImage(drawRegion, this->bgImage, imReg);
    }
}
