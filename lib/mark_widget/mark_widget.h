#ifndef MARK_WIDGET_H
#define MARK_WIDGET_H

#include <map>
#include <string>
#include <utility>
#include <vector>

#include <QEvent>
#include <QImage>
#include <QObject>
#include <QPainter>
#include <QPointF>
#include <QRectF>
#include <QSize>
#include <QSizeF>
#include <QWidget>

#include <mark_action.hpp>
#include <mark_instance.hpp>

class ImageView : public QWidget
{
    Q_OBJECT

   public:
    explicit ImageView(QWidget* parent = nullptr);
    virtual void reset(const QImage& image);

   protected:
    /** Member variables */
    QImage bgImage;      // Background Image
    QRectF viewRegion;   // View region on widget
    QRectF imageRegion;  // Image showing region on widget

    /** Region handling functions */
    QRectF find_image_region(const QPointF& center, const QSizeF& size);
    QRectF find_view_region(const QSize& sizeHint, const QSize& widgetSize);

    /** Point mapping functions */
    QPointF scaling_to_view(const QPointF& point);
    QSizeF scaling_to_view(const QSizeF& size);
    template <typename T>
    T scaling_to_view(const T& data)
    {
        QPointF factor(this->viewRegion.width() / this->imageRegion.width(),
                       this->viewRegion.height() / this->imageRegion.height());
        return data * factor;
    }

    QPointF mapping_to_view(const QPointF& point);
    QRectF mapping_to_view(const QRectF& rect);
    template <typename T>
    T mapping_to_view(const T& data)
    {
        return this->scaling_to_view<T>(data - this->imageRegion.topLeft()) +
               this->viewRegion.topLeft();
    }

    QPointF scaling_to_image(const QPointF& point);
    QSizeF scaling_to_image(const QSizeF& size);
    template <typename T>
    T scaling_to_image(const T& data)
    {
        QPointF factor(
            this->imageRegion.width() / this->viewRegion.width(),
            (this->imageRegion.height() / this->viewRegion.height()));
        return data * factor;
    }

    QPointF mapping_to_image(const QPointF& point);
    QRectF mapping_to_image(const QRectF& rect);
    template <typename T>
    T mapping_to_image(const T& point)
    {
        return this->scaling_to_image<T>(point - this->viewRegion.topLeft()) +
               this->imageRegion.topLeft();
    }
};

class ImageMap : public ImageView
{
    Q_OBJECT

   public:
    explicit ImageMap(QWidget* parent = nullptr);
    void reset(const QImage& image);
    const QRectF get_select_region();

   public slots:
    void set_select_region(const QRectF& selectRegion);

   signals:
    void selectRegionChanged(const QRectF& imageRegion);

   protected:
    /** Member variables */
    ical_mark::ClickAction clickAction;
    QRectF selectRegion;

    /** Event handler */
    bool event(QEvent* event);
    void paintEvent(QPaintEvent* paintEvent);
    void resizeEvent(QResizeEvent* event);

    /** Drawing functions */
    void draw_select_region(const QRectF& selRegion, const QRectF& viewRegion);
};

class RBoxMarkWidget : public ImageView
{
    Q_OBJECT

   public:
    explicit RBoxMarkWidget(QWidget* parent = nullptr);

    /** Initialization and setup */
    void reset(const QImage& image);
    void reset(const QImage& image,
               const std::vector<ical_mark::Instance>& instList);

    /** Data handling */
    int get_mark_label();
    int get_hl_instance_index();

    QRectF get_image_region();
    qreal get_scale_ratio();

    const std::vector<ical_mark::Instance>& annotation_list();
    void delete_instances(const std::vector<size_t>& indList);

   public slots:
    void set_class_names(const std::vector<std::string>& classNames);
    void set_mark_label(int label);
    void set_hl_instance_index(int index);  // Highlighting selected instance
    void set_image_region(const QRectF& imageRegion);
    void set_scale_ratio(qreal ratio);

    void move_image_region(int dx, int dy);

    void marking_revert();
    void marking_reset();

   signals:
    void markLabelChanged(int label);
    void hlInstanceIndexChanged(int index);
    void instanceListChanged(const std::vector<ical_mark::Instance>& annoList);
    void scaleRatioChanged(qreal ratio);
    void imageRegionChanged(const QRectF& imageRegion);

   protected:
    /** Style datatypes */
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
        int fontSize = 16;
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

    /** Member variables */
    ical_mark::RBoxMark markAction;
    ical_mark::ClickAction moveAction;

    QPointF mousePos;
    QPointF regionPosCache;
    qreal scaleRatio = 1.0;
    qreal scaleStep = 0.1;

    int label = 0;                              // Current marking label
    int highlightInst = -1;                     // Index for highlighting
    ical_mark::Instance curInst;                // Current marking instance
    std::vector<ical_mark::Instance> annoList;  // Marked instances
    std::vector<std::string> classNames;        // Class names

    Style style;  // Painting style

    /** Event handler */
    bool event(QEvent* event);
    void wheelEvent(QWheelEvent* event);
    void paintEvent(QPaintEvent* paintEvent);
    void resizeEvent(QResizeEvent* event);

    bool instance_marking(QEvent* event, bool& instListChanged);
    bool image_region_moving(QEvent* event, bool& imgRegionChanged);

    /** Region handling functions */
    using ImageView::find_image_region;
    QRectF find_image_region(const QPointF& center, const QSizeF& sizeHint,
                             qreal scaleRatio);
    bool update_regions(const QPointF& newCenter, qreal oldScaleRatio,
                        qreal newScaleRatio);
    bool update_scale_ratio(qreal newScaleRatio,
                            qreal* oldScaleRatioPtr = nullptr);

    /** Estimating functions */
    double find_distance(const QPointF& p1, const QPointF& p2);
    double find_degree(const QPointF& from, const QPointF& to);
    void fill_bbox(ical_mark::Instance& inst, const QPointF& pos1,
                   const QPointF& pos2);

    /** Drawing functions */
    void draw_aim_crosshair(const QPointF& center, double degree,
                            const StyleCrosshair& style);
    void draw_rotated_bbox(const ical_mark::Instance& inst,
                           const StyleRBox& style);
    void draw_anchor(const QPointF& pos, const StyleAnchor& style);
};

#endif  // MARKAREA_H
