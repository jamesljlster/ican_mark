#ifndef MARKAREA_H
#define MARKAREA_H

#include <map>
#include <string>
#include <vector>

#include <QEvent>
#include <QImage>
#include <QLine>
#include <QObject>
#include <QPaintDevice>
#include <QWidget>

#include <mark_action.hpp>

std::vector<QLine> draw_aim_crosshair(  //
    QPaintDevice* widget, const QPoint& center, float degree,
    const QColor& penColor = QColor(160, 160, 160));

class MarkArea : public QWidget
{
    Q_OBJECT
   public:
    explicit MarkArea(QWidget* parent = nullptr);

   signals:

   protected:
    // Member variables
    bool event(QEvent* event);
    void paintEvent(QPaintEvent* paintEvent);

    ical_mark::RBoxMark markAction;

    QImage bgImage;
    QPoint mousePos;

    float degree = 0;

    // Member functions
    float find_degree(const QPoint& from, const QPoint& to);
};

#endif  // MARKAREA_H
