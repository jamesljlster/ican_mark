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

#include <yaml-cpp/yaml.h>

#include <mark_action.hpp>

#define DEFAULT_PEN_COLOR QColor(160, 160, 160)

class RBoxMarkArea : public QWidget
{
    Q_OBJECT

   public:
    struct Instance
    {
        int label = 0;
        double degree = 0;
        double x = 0;
        double y = 0;
        double w = 0;
        double h = 0;

        bool valid() const;
        void reset();
        void reset_degree();
        void reset_bbox();
    };

    explicit RBoxMarkArea(QWidget* parent = nullptr);

   signals:

   protected:
    /** Member variables */
    ical_mark::RBoxMark markAction;

    QImage bgImage;
    QPoint mousePos;
    QPoint markBase;
    QSize markSize;

    Instance curInst;                // Current marking instance
    std::vector<Instance> annoList;  // Marked instances

    /** Event handler */
    bool event(QEvent* event);
    void paintEvent(QPaintEvent* paintEvent);

    /** Estimating functions */
    double find_distance(const QPointF& p1, const QPointF& p2);
    double find_degree(const QPoint& from, const QPoint& to);
    QRectF find_bbox(const QPoint& pos1, const QPoint& pos2, double degree);
    void fill_bbox(Instance& inst, const QPoint& pos1, const QPoint& pos2);

    /** Drawing functions */
    void draw_aim_crosshair(const QPoint& center, double degree,
                            const QColor& penColor = DEFAULT_PEN_COLOR);
    void draw_rotated_bbox(const Instance& inst, int ctrRad = 2,
                           const QColor& penColor = DEFAULT_PEN_COLOR);
    void draw_rotated_bbox(const QRectF& bbox, double degree, int ctrRad = 2,
                           const QColor& penColor = DEFAULT_PEN_COLOR);
};

#endif  // MARKAREA_H
