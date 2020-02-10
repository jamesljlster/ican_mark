#ifndef MARKWIDGET_H
#define MARKWIDGET_H

#include <map>
#include <string>
#include <vector>

#include <QEvent>
#include <QImage>
#include <QObject>
#include <QPainter>
#include <QRectF>
#include <QWidget>

#include <mark_action.hpp>
#include <mark_instance.hpp>

#define DEFAULT_PEN_COLOR QColor(160, 160, 160)
#define DEFAULT_BBOX_COLOR QColor(0, 0, 160, 160)

class RBoxMarkWidget : public QWidget
{
    Q_OBJECT

   public:
    struct StyleCrosshair
    {
        int lineWidth = 1;
        QPainter::RenderHint rendHint = QPainter::RenderHint::Antialiasing;
        QPainter::CompositionMode compMode =
            QPainter::RasterOp_SourceXorDestination;
        QColor penColor = QColor(160, 160, 160, 255);
    };

    struct StyleRBox
    {
        int centerRad = 0;  // Center point radius
        int lineWidth = 1;
        QPainter::RenderHint rendHint = QPainter::RenderHint::Antialiasing;
        QPainter::CompositionMode compMode =
            QPainter::CompositionMode_SourceOver;
        QColor penColor = QColor(0, 0, 160, 160);
    };

    struct StyleAnchor
    {
        int radius = 3;
        int lineWidth = 2;
        QPainter::RenderHint rendHint = QPainter::RenderHint::Antialiasing;
        QPainter::CompositionMode compMode =
            QPainter::CompositionMode_SourceOver;
        QColor penColor = QColor(0, 0, 0, 160);
    };

    struct Style
    {
        struct StyleCrosshair crosshair;
        struct StyleRBox rbox;
        struct StyleRBox rboxHL;  // Highlighted rotated bounding box
        struct StyleAnchor anchor;
    };

    explicit RBoxMarkWidget(QWidget* parent = nullptr);

    /** Initialization and setup */
    void reset(const QImage& image,
               const std::vector<ical_mark::Instance>& instList);
    void set_mark_label(int label);
    void set_hl_instance_index(int index);  // Highlighting selected instance

    /** Data handling */
    const std::vector<ical_mark::Instance>& annotation_list();
    void delete_instances(const std::vector<size_t>& indList);

   signals:
    void stateChanged(const std::vector<ical_mark::Instance>& annoList);

   protected:
    /** Member variables */
    ical_mark::RBoxMark markAction;

    QImage bgImage;
    QPoint mousePos;
    QPoint markBase;
    QSize markSize;

    int label = 0;                              // Current marking label
    int highlightInst = -1;                     // Index for highlighting
    ical_mark::Instance curInst;                // Current marking instance
    std::vector<ical_mark::Instance> annoList;  // Marked instances

    Style style;  // Painting style

    /** Event handler */
    bool event(QEvent* event);
    void paintEvent(QPaintEvent* paintEvent);

    /** Estimating functions */
    double find_distance(const QPointF& p1, const QPointF& p2);
    double find_degree(const QPoint& from, const QPoint& to);
    void fill_bbox(ical_mark::Instance& inst, const QPoint& pos1,
                   const QPoint& pos2);

    /** Drawing functions */
    void draw_aim_crosshair(const QPoint& center, double degree,
                            const StyleCrosshair& style);
    void draw_rotated_bbox(const ical_mark::Instance& inst,
                           const StyleRBox& style);
    void draw_anchor(const QPoint& pos, const StyleAnchor& style);
};

#endif  // MARKAREA_H
