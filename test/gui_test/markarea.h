#ifndef MARKAREA_H
#define MARKAREA_H

#include <map>
#include <string>

#include <QEvent>
#include <QImage>
#include <QObject>
#include <QPaintDevice>
#include <QWidget>

#include <mark_action.hpp>

void draw_aim_crosshair(QPaintDevice* widget, const QPoint& center,
                        float degree,
                        const QColor& penColor = QColor(160, 160, 160));

class MarkArea : public QWidget
{
    Q_OBJECT
   public:
    explicit MarkArea(QWidget* parent = nullptr);

   signals:

   protected:
    bool event(QEvent* event);
    void paintEvent(QPaintEvent* paintEvent);

    ical_mark::RBoxMark markAction;

    QImage bgImage;
    QPoint mousePos;

    float degree = 0;
};

#endif  // MARKAREA_H
