#include "mark_widget.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>

#include <QBrush>
#include <QFont>
#include <QKeyEvent>
#include <QMarginsF>
#include <QMouseEvent>
#include <QPaintEvent>
#include <Qt>
#include <QtMath>

using namespace std;
using namespace ical_mark;

RBoxMarkWidget::RBoxMarkWidget(QWidget* parent) : ImageView(parent)
{
    this->setMouseTracking(true);
    this->setCursor(Qt::BlankCursor);

    // Set default painting style
    this->style.rboxHL.lineWidth = 2;
    this->style.rboxHL.centerRad = 3;
}

void RBoxMarkWidget::reset(const QImage& image)
{
    this->reset(image, vector<Instance>());
}

void RBoxMarkWidget::reset(const QImage& image,
                           const vector<Instance>& instList)
{
    bool imgRegionChanged = false;

    // Call parent reset function
    ImageView::reset(image);

    // Reset marking state
    this->annoList = instList;
    this->markAction.reset();
    this->moveAction.reset();

    // Reset view and image region
    imgRegionChanged = this->update_regions(
        QPointF(this->bgImage.width() / 2, this->bgImage.height() / 2),
        this->scaleRatio, this->scaleRatio);

    // Repaint and raise signal
    this->repaint();
    emit instanceListChanged(this->annoList);
    if (imgRegionChanged) emit imageRegionChanged(this->imageRegion);
}

void RBoxMarkWidget::set_class_names(const std::vector<std::string>& classNames)
{
    this->classNames = classNames;
}

int RBoxMarkWidget::get_mark_label() { return this->label; }
void RBoxMarkWidget::set_mark_label(int label)
{
    if (this->label != label)
    {
        this->label = label;
        this->repaint();
        emit markLabelChanged(this->label);
    }
}

int RBoxMarkWidget::get_hl_instance_index() { return this->highlightInst; }
void RBoxMarkWidget::set_hl_instance_index(int index)
{
    if (this->highlightInst != index)
    {
        this->highlightInst = index;
        this->repaint();
        emit hlInstanceIndexChanged(this->highlightInst);
    }
}

qreal RBoxMarkWidget::get_scale_ratio() { return this->scaleRatio; }
void RBoxMarkWidget::set_scale_ratio(qreal ratio)
{
    qreal oldScaleRatio;
    bool imgRegionChanged = false;
    bool scaleChanged = this->update_scale_ratio(ratio, &oldScaleRatio);

    if (scaleChanged)
    {
        // Update regions
        imgRegionChanged = this->update_regions(this->imageRegion.center(),
                                                oldScaleRatio, ratio);
    }

    // Repaint and raise signals
    this->repaint();
    if (imgRegionChanged) emit imageRegionChanged(this->imageRegion);
    if (scaleChanged) emit scaleRatioChanged(this->scaleRatio);
}

QRectF RBoxMarkWidget::get_image_region() { return this->imageRegion; }
void RBoxMarkWidget::set_image_region(const QRectF& imageRegion)
{
    bool imgRegionChanged = false;
    imgRegionChanged = this->update_regions(imageRegion.center(),
                                            this->scaleRatio, this->scaleRatio);

    this->repaint();
    if (imgRegionChanged) emit imageRegionChanged(this->imageRegion);
}

void RBoxMarkWidget::move_image_region(int dx, int dy)
{
    QRectF imgRegion = this->get_image_region();
    imgRegion.moveTo(imgRegion.topLeft() + QPoint(dx, dy));
    this->set_image_region(imgRegion);
}

void RBoxMarkWidget::marking_revert()
{
    this->markAction.revert();
    this->repaint();
}

void RBoxMarkWidget::marking_reset()
{
    this->markAction.reset();
    this->repaint();
}

const vector<Instance>& RBoxMarkWidget::annotation_list()
{
    return this->annoList;
}

void RBoxMarkWidget::delete_instances(const vector<size_t>& indList)
{
    vector<size_t> indSort = indList;
    sort(indSort.begin(), indSort.end());
    for (auto i = indSort.rbegin(); i != indSort.rend(); i++)
    {
        if ((int)*i == this->highlightInst)
        {
            this->highlightInst = -1;
        }

        this->annoList.erase(this->annoList.begin() + *i);
    }

    this->repaint();
    emit instanceListChanged(this->annoList);
}

bool RBoxMarkWidget::event(QEvent* event)
{
    bool ret = false;
    bool imgRegionChanged = false;
    bool instListChanged = false;

    ret |= this->instance_marking(event, instListChanged);
    ret |= this->image_region_moving(event, imgRegionChanged);

    if (ret)
    {
        this->repaint();

        // Raise signals
        if (imgRegionChanged) emit imageRegionChanged(this->imageRegion);
        if (instListChanged) emit instanceListChanged(this->annoList);

        return ret;
    }
    else
    {
        return QWidget::event(event);
    }
}

void RBoxMarkWidget::wheelEvent(QWheelEvent* event)
{
    bool imgRegionChanged = false;

    // Find new scale ratio
    qreal newScaleRatio = this->scaleRatio + (event->angleDelta().y() /
                                              abs(event->angleDelta().y())) *
                                                 this->scaleStep;

    qreal oldScaleRatio;
    bool scaleChanged = this->update_scale_ratio(newScaleRatio, &oldScaleRatio);
    if (scaleChanged)
    {
        // Find new center of image region
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        QPointF wheelPos = this->mapping_to_image(event->posF());
#else
        QPointF wheelPos = this->mapping_to_image(event->position());
#endif
        QPointF center = this->imageRegion.center();
        QPointF newCenter =
            ((center - wheelPos) * oldScaleRatio) / newScaleRatio + wheelPos;

        // Update regions
        imgRegionChanged =
            this->update_regions(newCenter, oldScaleRatio, newScaleRatio);
    }

    // Raise signal
    if (scaleChanged) emit scaleRatioChanged(this->scaleRatio);
    if (imgRegionChanged) emit imageRegionChanged(this->imageRegion);
}

void RBoxMarkWidget::paintEvent(QPaintEvent* paintEvent)
{
    int width = this->width();
    int height = this->height();

    (void)paintEvent;

    // Paint background
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setBrush(QBrush(QColor(0, 0, 0), Qt::SolidPattern));
    painter.drawRect(0, 0, width, height);

    if (!this->bgImage.isNull())
    {
        painter.drawImage(this->viewRegion, this->bgImage, this->imageRegion);
    }

    // Draw aim crosshair
    this->draw_aim_crosshair(this->mousePos, this->curInst.degree,
                             this->style.crosshair);
    if (static_cast<RBoxMark::State>(this->markAction.state()) ==
        RBoxMark::State::INIT)
    {
        if (static_cast<TwiceClick::State>(
                this->markAction["degree"].state()) ==
            TwiceClick::State::POS1_FIN)
        {
            this->draw_anchor(this->markAction["degree"]["pos1"]["release"],
                              this->style.anchor);
        }
    }

    // Draw marked instances
    for (int i = 0; i < (int)this->annoList.size(); i++)
    {
        const Instance& anno = this->annoList[i];
        if (i == this->highlightInst)
        {
            this->draw_rotated_bbox(anno, this->style.rboxHL);
        }
        else
        {
            this->draw_rotated_bbox(anno, this->style.rbox);
        }
    }

    // Draw rbox marking progress
    if (static_cast<RBoxMark::State>(this->markAction.state()) ==
        RBoxMark::State::DEGREE_FIN)
    {
        if (this->curInst.valid())
        {
            this->draw_rotated_bbox(this->curInst, this->style.rbox);
        }
    }
}

void RBoxMarkWidget::resizeEvent(QResizeEvent* event)
{
    bool imgRegionChanged = false;
    imgRegionChanged = this->update_regions(this->imageRegion.center(),
                                            this->scaleRatio, this->scaleRatio);
    if (imgRegionChanged) emit imageRegionChanged(this->imageRegion);
}

bool RBoxMarkWidget::instance_marking(QEvent* event, bool& instListChanged)
{
    bool ret = false;
    QEvent::Type eventType = event->type();

    // Run FSM of marking action
    if (eventType == QEvent::MouseMove)
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        this->mousePos = me->pos();
        this->markAction.run(me);
        ret = true;
    }
    else if (eventType == QEvent::MouseButtonPress ||
             eventType == QEvent::MouseButtonRelease)
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::MouseButton::LeftButton)
        {
            this->markAction.run(me);
            ret = true;
        }
    }

    // Status checking and processing
    switch (static_cast<RBoxMark::State>(this->markAction.state()))
    {
        case RBoxMark::State::INIT:

            if (static_cast<TwiceClick::State>(
                    this->markAction["degree"].state()) ==
                TwiceClick::State::POS1_FIN)
            {
                // Calculate and set degree
                this->curInst.degree = this->find_degree(
                    this->mapping_to_image(
                        this->markAction["degree"]["pos1"]["release"]),
                    this->mapping_to_image(this->mousePos));
            }
            else
            {
                // Reset degree
                this->curInst.degree = 0;
            }

            break;

        case RBoxMark::State::DEGREE_FIN:

            if (static_cast<TwiceClick::State>(
                    this->markAction["bbox"].state()) ==
                TwiceClick::State::POS1_FIN)
            {
                // Fill bounding box
                this->fill_bbox(
                    this->curInst,
                    this->mapping_to_image(
                        this->markAction["bbox"]["pos1"]["release"]),
                    this->mapping_to_image(this->mousePos));
            }
            else
            {
                // Reset bounding box
                this->curInst.reset_bbox();
            }

            break;

        case RBoxMark::State::BBOX_FIN:

            // Set label
            this->curInst.label = this->label;

            // Append instance to annotation list
            this->annoList.push_back(this->curInst);
            instListChanged = true;

            this->curInst.reset();
            this->markAction.reset();

            break;
    }

    return ret;
}

bool RBoxMarkWidget::image_region_moving(QEvent* event, bool& imgRegionChanged)
{
    bool ret = false;
    QEvent::Type eventType = event->type();

    // Run FSM of moving action
    if (eventType == QEvent::MouseMove)
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        this->mousePos = me->pos();
        this->moveAction.run(me);
        ret = true;
    }
    else if (eventType == QEvent::MouseButtonPress ||
             eventType == QEvent::MouseButtonRelease)
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::MouseButton::MiddleButton)
        {
            this->moveAction.run(me);
            ret = true;
        }
    }

    // Status checking and processing
    switch (static_cast<ClickAction::State>(this->moveAction.state()))
    {
        case ClickAction::State::MOVE:
            this->regionPosCache = this->imageRegion.center();
            break;

        case ClickAction::State::PRESS:
            imgRegionChanged = this->update_regions(
                this->regionPosCache +
                    this->scaling_to_image(this->moveAction["press"] -
                                           this->moveAction["move"]),
                this->scaleRatio, this->scaleRatio);
            break;

        case ClickAction::State::RELEASE:
            this->moveAction.reset();
            break;
    }

    return ret;
}

QRectF RBoxMarkWidget::find_image_region(const QPointF& center,
                                         const QSizeF& sizeHint,
                                         qreal scaleRatio)
{
    QSizeF size =
        sizeHint.scaled(this->bgImage.size(), Qt::KeepAspectRatioByExpanding) /
        scaleRatio;
    return this->find_image_region(center, size);
}

bool RBoxMarkWidget::update_regions(const QPointF& newCenter,
                                    qreal oldScaleRatio, qreal newScaleRatio)
{
    bool imgRegionChanged = false;

    // Mapping mark points to image space
    RBoxMark imgMark = this->mapping_to_image<RBoxMark>(this->markAction);

    // Update image region
    QRectF newImageRegion =
        this->find_image_region(newCenter, this->size(), newScaleRatio);
    if (this->imageRegion != newImageRegion)
    {
        // Assign new image region
        this->imageRegion = newImageRegion;
        imgRegionChanged = true;
    }

    // Update view region
    this->viewRegion =
        this->find_view_region(this->imageRegion.size().toSize(), this->size());

    // Mapping mark points back to view space
    this->markAction = this->mapping_to_view<RBoxMark>(imgMark);

    return imgRegionChanged;
}

bool RBoxMarkWidget::update_scale_ratio(qreal newScaleRatio,
                                        qreal* oldScaleRatioPtr)
{
    if (oldScaleRatioPtr)
    {
        *oldScaleRatioPtr = this->scaleRatio;
    }

    if (newScaleRatio < 1.0)
    {
        newScaleRatio = 1.0;
    }

    if (this->scaleRatio != newScaleRatio)
    {
        this->scaleRatio = newScaleRatio;
        return true;
    }
    else
    {
        return false;
    }
}

double RBoxMarkWidget::find_distance(const QPointF& p1, const QPointF& p2)
{
    qreal pwrDist = qPow(p1.x() - p2.x(), 2) + qPow(p1.y() - p2.y(), 2);
    if (pwrDist > 0)
    {
        return qSqrt(pwrDist);
    }
    else
    {
        return 0;
    }
}

double RBoxMarkWidget::find_degree(const QPointF& from, const QPointF& to)
{
    qreal theta = qAtan2(from.y() - to.y(), to.x() - from.x());
    theta = qRadiansToDegrees(theta) - 90.0;
    if (theta < -180.0)
    {
        theta += 360.0;
    }

    return theta;
}

void RBoxMarkWidget::fill_bbox(Instance& inst, const QPointF& pos1,
                               const QPointF& pos2)
{
    double w, h;
    double degree = inst.degree;

    QLineF line1, line2;
    QPointF crossPt1, crossPt2, center;

    // Set center point of lines
    line1.setP1(pos1);
    line2.setP1(pos2);

    // Find cross point 1 and height
    line1.setAngle(degree);
    line2.setAngle(degree + 90);

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    line1.intersect(line2, &crossPt1);
#else
    line1.intersects(line2, &crossPt1);
#endif
    h = this->find_distance(pos2, crossPt1);

    // Find cross point 2 and width
    line1.setAngle(degree + 90);
    line2.setAngle(degree);

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    line1.intersect(line2, &crossPt2);
#else
    line1.intersects(line2, &crossPt2);
#endif
    w = this->find_distance(pos2, crossPt2);

    // Find center point
    line1.setPoints(pos1, pos2);
    line2.setPoints(crossPt1, crossPt2);
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    line1.intersect(line2, &center);
#else
    line1.intersects(line2, &center);
#endif

    inst.x = center.x();
    inst.y = center.y();
    inst.w = w;
    inst.h = h;
}

void RBoxMarkWidget::draw_aim_crosshair(const QPointF& center, double degree,
                                        const StyleCrosshair& style)
{
    int width = this->width();
    int height = this->height();

    // Setup painter and drawing style
    QPainter painter(this);

    painter.setRenderHint(style.rendHint);
    painter.setCompositionMode(style.compMode);
    painter.setPen(QPen(style.penColor, style.lineWidth));

    // Find lines of crosshair
    qreal shift = fmod(degree + 225, 90) - 45;
    shift = qTan(qDegreesToRadians(shift));

    QLineF line1(  //
        QPointF(center.x() - shift * center.y(), 0),
        QPointF(center.x() + shift * (height - center.y()), height));
    QLineF line2(  //
        QPointF(0, center.y() + shift * center.x()),
        QPointF(width, center.y() - shift * (width - center.x())));

    // Draw aim crosshair
    painter.drawLine(line1);
    painter.drawLine(line2);
}

void RBoxMarkWidget::draw_rotated_bbox(const Instance& inst,
                                       const StyleRBox& style)
{
    // Setup painter and drawing style
    QPainter painter(this);

    painter.setRenderHint(style.rendHint);
    painter.setCompositionMode(style.compMode);
    painter.setPen(QPen(style.penColor, style.lineWidth));

    // Draw center point
    QPointF center = this->mapping_to_view(QPointF(inst.x, inst.y));
    if (style.centerRad > 0)
    {
        painter.drawEllipse(center, style.centerRad, style.centerRad);
    }

    // Draw rotated bounding box
    double halfWidth = inst.w / 2;
    double halfHeight = inst.h / 2;

    QTransform transform;
    transform.translate(center.x(), center.y());
    transform.rotate(-inst.degree);
    painter.setTransform(transform);

    QRectF rect =
        QRectF(this->scaling_to_view(QPointF(-halfWidth, -halfHeight)),
               this->scaling_to_view(QPointF(halfWidth, halfHeight)));
    painter.drawRect(rect);

    // Draw label
    QFont font = painter.font();
    font.setPixelSize(style.fontSize);
    font.setBold(true);
    painter.setFont(font);

    string labelStr = to_string(inst.label);
    if (inst.label < (int)this->classNames.size())
    {
        labelStr += string(": ") + this->classNames[inst.label];
    }

    painter.drawText(rect.marginsRemoved(QMarginsF(5, 5, 5, 5)),
                     Qt::AlignTop | Qt::AlignTop, labelStr.c_str());
}

void RBoxMarkWidget::draw_anchor(const QPointF& pos, const StyleAnchor& style)
{
    // Setup painter and drawing style
    QPainter painter(this);

    painter.setRenderHint(style.rendHint);
    painter.setCompositionMode(style.compMode);
    painter.setPen(QPen(style.penColor, style.lineWidth));

    // Draw anchor point
    painter.drawEllipse(pos, style.radius, style.radius);
}
