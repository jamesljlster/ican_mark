#ifndef __IMAGEVIEWTEST_H__
#define __IMAGEVIEWTEST_H__

#include <QImage>
#include <QPainter>
#include <QResizeEvent>

#include <mark_widget.h>

class ImageViewTest : public ImageView
{
    Q_OBJECT

   public:
    explicit ImageViewTest(QWidget* parent = nullptr) : ImageView(parent)
    {
        this->bgImage = QImage("color_map.png");
        this->viewScale = 1.0;
        this->viewCenter =
            QPointF(this->bgImage.width() / 2, this->bgImage.height() / 2);
    }

   protected:
    void paintEvent(QPaintEvent* paintEvent)
    {
        (void)paintEvent;
        this->draw_background();
    }

    void resizeEvent(QResizeEvent* event) { this->zoom_to_fit(); }
};

#endif
