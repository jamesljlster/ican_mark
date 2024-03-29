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

    /** View handling functions */
    virtual void zoom_to_fit();

   protected:
    /** Member variables */
    QImage bgImage;      // Background image
    QPointF viewCenter;  // The center point of background image on view space
    double viewScale = 1.0;  // Scaling ratio of view

    QImage scaledImg;          // Scaled background image
    double currentScale = -1;  // Scale ratio corresponding to scaledImg

    /** View handling functions */
    double find_fit_scale_ratio() const;
    QPointF find_centered_point() const;

    /** Point mapping functions */
    QPointF scaling_to_view(const QPointF& point);
    QSizeF scaling_to_view(const QSizeF& size);
    template <typename T>
    T scaling_to_view(const T& data)
    {
        return data * this->viewScale;
    }

    QPointF mapping_to_view(const QPointF& point);
    QRectF mapping_to_view(const QRectF& rect);
    template <typename T>
    T mapping_to_view(const T& data)
    {
        QPointF imgCtr =
            QPointF(this->bgImage.width(), this->bgImage.height()) / 2;
        return this->scaling_to_view<T>(data - imgCtr) + this->viewCenter;
    }

    QPointF scaling_to_image(const QPointF& point);
    QSizeF scaling_to_image(const QSizeF& size);
    template <typename T>
    T scaling_to_image(const T& data)
    {
        return data / this->viewScale;
    }

    QPointF mapping_to_image(const QPointF& point);
    QRectF mapping_to_image(const QRectF& rect);
    template <typename T>
    T mapping_to_image(const T& point)
    {
        QPointF imgCtr =
            QPointF(this->bgImage.width(), this->bgImage.height()) / 2;
        return this->scaling_to_image<T>(point - this->viewCenter) + imgCtr;
    }

    /** Default drawing functions */
    void draw_background(const QColor& bgColor = QColor(0, 0, 0));
};

class ImageMap : public ImageView
{
    Q_OBJECT

   public:
    explicit ImageMap(QWidget* parent = nullptr);
    void reset(const QImage& image);

   public slots:
    void set_select_region(const QRectF& selectRegion);

   signals:
    void selectCenterChanged(const QPointF& selCenter);

   protected:
    /** Member variables */
    ican_mark::ClickAction clickAction;
    QRectF selectRegion;

    /** Event handler */
    bool event(QEvent* event);
    void paintEvent(QPaintEvent* paintEvent);
    void resizeEvent(QResizeEvent* event);

    /** Drawing functions */
    void draw_select_region();
};

class RBoxMarkWidget : public ImageView
{
    Q_OBJECT

   public:
    explicit RBoxMarkWidget(QWidget* parent = nullptr);

    /** Initialization and setup */
    void reset(const QImage& image);
    void reset(const QImage& image,
               const std::vector<ican_mark::Instance>& instList);

    /** View handling functions */
    void zoom_to_fit();

    /** Data handling */
    int get_mark_label();
    int get_hl_instance_index();

    qreal get_scale_ratio();

    const std::vector<ican_mark::Instance>& annotation_list();
    void delete_instances(const std::vector<size_t>& indList);

   public slots:
    void set_class_names(const std::vector<std::string>& classNames);
    void set_mark_label(int label);
    void set_hl_instance_index(int index);  // Highlighting selected instance
    void set_view_center(const QPointF& viewCenter);
    void set_select_center(const QPointF& viewCenter);
    void set_scale_ratio(qreal ratio);

    void move_view_region(int dx, int dy);

    void marking_revert();
    void marking_reset();

   signals:
    void markLabelChanged(int label);
    void hlInstanceIndexChanged(int index);
    void instanceListChanged(const std::vector<ican_mark::Instance>& annoList);
    void scaleRatioChanged(qreal ratio);
    void selectRegionChanged(const QRectF& selRegion);
    void viewCenterChanged(const QPointF& viewCenter);

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
    ican_mark::RBoxMark markAction;
    ican_mark::ClickAction moveAction;

    QPointF mousePos;
    QPointF viewCtrCache;
    QRectF selRegion;  // Select region on image space

    qreal scaleStep = 0.1;
    qreal scaleMin = 1e-4;

    int label = 0;                              // Current marking label
    int highlightInst = -1;                     // Index for highlighting
    ican_mark::Instance curInst;                // Current marking instance
    std::vector<ican_mark::Instance> annoList;  // Marked instances
    std::vector<std::string> classNames;        // Class names

    Style style;  // Painting style

    /** Event handler */
    bool event(QEvent* event);
    void wheelEvent(QWheelEvent* event);
    void paintEvent(QPaintEvent* paintEvent);
    void resizeEvent(QResizeEvent* event);

    bool instance_marking(QEvent* event, bool& instListChanged);
    bool image_region_moving(QEvent* event, bool& viewCtrChanged);

    /** View handling functions */
    bool update_select_region();
    bool update_view_center(const QPointF& newCenter);
    bool update_scale_ratio(qreal newScaleRatio,
                            qreal* oldScaleRatioPtr = nullptr);

    /** Estimating functions */
    double find_distance(const QPointF& p1, const QPointF& p2);
    double find_degree(const QPointF& from, const QPointF& to);
    void fill_bbox(ican_mark::Instance& inst, const QPointF& pos1,
                   const QPointF& pos2);

    /** Drawing functions */
    void draw_aim_crosshair(const QPointF& center, double degree,
                            const StyleCrosshair& style);
    void draw_rotated_bbox(const ican_mark::Instance& inst,
                           const StyleRBox& style);
    void draw_anchor(const QPointF& pos, const StyleAnchor& style);

    /** Instance handling */
    bool inst_valid(const ican_mark::Instance& inst);
    void inst_reset_bbox(ican_mark::Instance& inst);
    void inst_reset(ican_mark::Instance& inst);
};

#endif  // MARKAREA_H
