#ifndef MARKAREA_H
#define MARKAREA_H

#include <map>
#include <string>
#include <vector>

#include <QEvent>
#include <QImage>
#include <QObject>
#include <QRectF>
#include <QWidget>

#include <mark_action.hpp>

#define DEFAULT_PEN_COLOR QColor(160, 160, 160)

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
    QPoint markBase;
    QSize markSize;

    float degree = 0;
    QRectF bbox;

    // Member functions
    float find_distance(const QPointF& p1, const QPointF& p2);
    float find_degree(const QPoint& from, const QPoint& to);
    QRectF find_bbox(const QPoint& pos1, const QPoint& pos2, float degree);

    void draw_aim_crosshair(const QPoint& center, float degree,
                            const QColor& penColor = DEFAULT_PEN_COLOR);
    void draw_rotated_bbox(const QRectF& bbox, float degree, int ctrRad = 2,
                           const QColor& penColor = DEFAULT_PEN_COLOR);
};

#endif  // MARKAREA_H
