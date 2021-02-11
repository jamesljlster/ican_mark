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
using namespace ican_mark;

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
    // Call parent reset function
    ImageView::reset(image);

    // Update view region
    this->update_select_region();

    // Reset marking state
    this->annoList = instList;
    this->markAction.reset();
    this->moveAction.reset();

    // Repaint and raise signal
    this->repaint();

    emit instanceListChanged(this->annoList);
    emit scaleRatioChanged(this->viewScale);
    emit viewCenterChanged(this->viewCenter);
    emit selectRegionChanged(this->selRegion);
}

void RBoxMarkWidget::zoom_to_fit()
{
    ImageView::zoom_to_fit();
    this->repaint();

    emit scaleRatioChanged(this->viewScale);
    emit viewCenterChanged(this->viewCenter);
    emit selectRegionChanged(this->selRegion);
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

qreal RBoxMarkWidget::get_scale_ratio() { return this->viewScale; }
void RBoxMarkWidget::set_scale_ratio(qreal ratio)
{
    qreal oldScaleRatio;
    bool viewCtrChanged = false;
    bool selRegionChanged = false;
    bool scaleChanged = this->update_scale_ratio(ratio, &oldScaleRatio);

    // Update view center and regions
    if (scaleChanged)
    {
        viewCtrChanged = this->update_view_center(this->viewCenter);
        selRegionChanged = this->update_select_region();
    }

    // Repaint and raise signals
    this->repaint();

    if (scaleChanged) emit scaleRatioChanged(this->viewScale);
    if (viewCtrChanged) emit viewCenterChanged(this->viewCenter);
    if (selRegionChanged) emit selectRegionChanged(this->selRegion);
}

void RBoxMarkWidget::set_view_center(const QPointF& viewCenter)
{
    bool viewCtrChanged = this->update_view_center(viewCenter);
    bool selRegionChanged = this->update_select_region();

    this->repaint();
    if (viewCtrChanged) emit viewCenterChanged(this->viewCenter);
    if (selRegionChanged) emit selectRegionChanged(this->selRegion);
}

void RBoxMarkWidget::set_select_center(const QPointF& selCenter)
{
    this->set_view_center(this->mapping_to_view(
        (QPointF(this->bgImage.width(), this->bgImage.height()) / 2) -
        selCenter +
        this->mapping_to_image(QPointF(this->width(), this->height()) / 2)));
}

void RBoxMarkWidget::move_view_region(int dx, int dy)
{
    this->set_view_center(this->viewCenter + QPointF(dx, dy));
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
    if (indList.size())
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
}

bool RBoxMarkWidget::event(QEvent* event)
{
    bool ret = false;
    bool viewCtrChanged = false;
    bool selRegionChanged = false;
    bool instListChanged = false;

    ret |= this->instance_marking(event, instListChanged);
    ret |= this->image_region_moving(event, viewCtrChanged);

    selRegionChanged = this->update_select_region();

    if (ret)
    {
        this->repaint();

        // Raise signals
        if (viewCtrChanged) emit viewCenterChanged(this->viewCenter);
        if (selRegionChanged) emit selectRegionChanged(this->selRegion);
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
    bool viewCtrChanged = false;
    bool selRegionChanged = false;

    // Find new scale ratio
    qreal newScaleRatio = this->viewScale + (event->angleDelta().y() /
                                             abs(event->angleDelta().y())) *
                                                this->scaleStep;

    qreal oldScaleRatio;
    bool scaleChanged = this->update_scale_ratio(newScaleRatio, &oldScaleRatio);

    // Update view region
    if (scaleChanged)
    {
        // Find new center of view region
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        QPointF wheelPos = event->posF();
#else
        QPointF wheelPos = event->position();
#endif

        QPointF center = this->viewCenter;
        QPointF newCenter =
            ((center - wheelPos) * this->viewScale) / oldScaleRatio + wheelPos;

        // Update view regions
        viewCtrChanged = this->update_view_center(newCenter);
        selRegionChanged = this->update_select_region();
    }

    // Raise signal
    if (scaleChanged) emit scaleRatioChanged(this->viewScale);
    if (viewCtrChanged) emit viewCenterChanged(this->viewCenter);
    if (selRegionChanged) emit selectRegionChanged(this->selRegion);
}

void RBoxMarkWidget::paintEvent(QPaintEvent* paintEvent)
{
    (void)paintEvent;

    // Paint background
    this->draw_background();

    // Draw aim crosshair
    double instDegree =
        this->curInst.has_degree() ? this->curInst.get_degree() : 0;
    this->draw_aim_crosshair(this->mousePos, instDegree, this->style.crosshair);
    if (static_cast<RBoxMark::State>(this->markAction.state()) ==
        RBoxMark::State::INIT)
    {
        if (static_cast<TwiceClick::State>(
                this->markAction["degree"].state()) ==
            TwiceClick::State::POS1_FIN)
        {
            this->draw_anchor(
                this->mapping_to_view(
                    this->markAction["degree"]["pos1"]["release"]),
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
        if (this->inst_valid(this->curInst))
        {
            this->draw_rotated_bbox(this->curInst, this->style.rbox);
        }
    }
}

void RBoxMarkWidget::resizeEvent(QResizeEvent* event)
{
    QSize size = event->size();
    QSize oldSize = event->oldSize();
    this->move_view_region((double)(size.width() - oldSize.width()) / 2.0,
                           (double)(size.height() - oldSize.height()) / 2.0);
}

bool RBoxMarkWidget::instance_marking(QEvent* event, bool& instListChanged)
{
    bool ret = false;
    QEvent::Type eventType = event->type();

    // Set label
    this->curInst.set_label(this->label);

    // Run FSM of marking action
    if (eventType == QEvent::MouseMove)
    {
        QMouseEvent me = *(static_cast<QMouseEvent*>(event));
        this->mousePos = me.pos();

        me.setLocalPos(this->mapping_to_image(me.localPos()));
        this->markAction.run(&me);
        ret = true;
    }
    else if (eventType == QEvent::MouseButtonPress ||
             eventType == QEvent::MouseButtonRelease)
    {
        QMouseEvent me = *(static_cast<QMouseEvent*>(event));
        if (me.button() == Qt::MouseButton::LeftButton)
        {
            me.setLocalPos(this->mapping_to_image(me.localPos()));
            this->markAction.run(&me);
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
                this->curInst.set_degree(this->find_degree(
                    this->markAction["degree"]["pos1"]["release"],
                    this->mapping_to_image(this->mousePos)));
            }
            else
            {
                // Reset degree
                this->curInst.clear_degree();
            }

            break;

        case RBoxMark::State::DEGREE_FIN:

            if (static_cast<TwiceClick::State>(
                    this->markAction["bbox"].state()) ==
                TwiceClick::State::POS1_FIN)
            {
                // Fill bounding box
                this->fill_bbox(this->curInst,
                                this->markAction["bbox"]["pos1"]["release"],
                                this->mapping_to_image(this->mousePos));
            }
            else
            {
                // Reset bounding box
                this->inst_reset_bbox(this->curInst);
            }

            break;

        case RBoxMark::State::BBOX_FIN:

            // Append instance to annotation list
            this->annoList.push_back(this->curInst);
            instListChanged = true;

            this->inst_reset(this->curInst);
            this->markAction.reset();

            break;
    }

    return ret;
}

bool RBoxMarkWidget::image_region_moving(QEvent* event, bool& viewCtrChanged)
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
            this->viewCtrCache = this->viewCenter;
            break;

        case ClickAction::State::PRESS:
            viewCtrChanged = this->update_view_center(
                this->viewCtrCache -
                (this->moveAction["press"] - this->moveAction["move"]));
            break;

        case ClickAction::State::RELEASE:
            this->moveAction.reset();
            break;
    }

    return ret;
}

bool RBoxMarkWidget::update_select_region()
{
    QRectF newSelRegion =
        QRectF(QPointF(0, 0), QSizeF(this->size()) / this->viewScale);
    newSelRegion.moveCenter(
        this->mapping_to_image(QPointF(this->width(), this->height()) / 2));

    if (this->selRegion != newSelRegion)
    {
        this->selRegion = newSelRegion;
        return true;
    }
    else
    {
        return false;
    }
}

bool RBoxMarkWidget::update_view_center(const QPointF& newCenter)
{
    bool viewCtrChanged = false;

    // Update view center
    QPointF tmpCenter = newCenter;

    int width = this->width();
    int height = this->height();

    QPointF halfImSize = this->scaling_to_view(QPointF(
                             this->bgImage.width(), this->bgImage.height())) /
                         2.0;
    int halfImWidth = halfImSize.x();
    int halfImHeight = halfImSize.y();
    int minRsvSize = min(width / 2, height / 2);  // Minimum reserved size

    int tmpSize;

    tmpSize = minRsvSize - halfImWidth;
    if (tmpCenter.x() < tmpSize) tmpCenter.setX(tmpSize);

    tmpSize = minRsvSize - halfImHeight;
    if (tmpCenter.y() < tmpSize) tmpCenter.setY(tmpSize);

    tmpSize = width + halfImWidth - minRsvSize;
    if (tmpCenter.x() > tmpSize) tmpCenter.setX(tmpSize);

    tmpSize = height + halfImHeight - minRsvSize;
    if (tmpCenter.y() > tmpSize) tmpCenter.setY(tmpSize);

    if (this->viewCenter != tmpCenter)
    {
        viewCtrChanged = true;
        this->viewCenter = tmpCenter;
    }

    return viewCtrChanged;
}

bool RBoxMarkWidget::update_scale_ratio(qreal newScaleRatio,
                                        qreal* oldScaleRatioPtr)
{
    if (oldScaleRatioPtr)
    {
        *oldScaleRatioPtr = this->viewScale;
    }

    if (newScaleRatio < this->scaleMin)
    {
        newScaleRatio = this->scaleMin;
    }

    if (this->viewScale != newScaleRatio)
    {
        this->viewScale = newScaleRatio;
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
    double degree = inst.get_degree();

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

    inst.set_x(center.x());
    inst.set_y(center.y());
    inst.set_w(w);
    inst.set_h(h);
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
    QPointF center = this->mapping_to_view(QPointF(inst.get_x(), inst.get_y()));
    if (style.centerRad > 0)
    {
        painter.drawEllipse(center, style.centerRad, style.centerRad);
    }

    // Draw rotated bounding box
    double degree = inst.has_degree() ? inst.get_degree() : 0;
    double halfWidth = inst.get_w() / 2;
    double halfHeight = inst.get_h() / 2;

    QTransform transform;
    transform.translate(center.x(), center.y());
    transform.rotate(-degree);
    painter.setTransform(transform);

    QRectF boxRect(this->scaling_to_view(QPointF(-halfWidth, -halfHeight)),
                   this->scaling_to_view(QPointF(halfWidth, halfHeight)));
    painter.drawRect(boxRect);

    // Draw label
    int instLabel = inst.get_label();
    string labelStr = to_string(instLabel);
    if (instLabel < (int)this->classNames.size())
    {
        labelStr += string(": ") + this->classNames[instLabel];
    }

    QRectF labelRect(
        boxRect.topLeft() + QPoint(style.lineWidth / 2, style.lineWidth / 2),
        QSizeF(boxRect.width() - style.lineWidth, style.fontSize + 15));

    QFont font = painter.font();
    font.setPixelSize(style.fontSize);
    font.setBold(true);
    painter.setFont(font);

    painter.fillRect(labelRect, QColor(255, 255, 255, 64));
    painter.drawText(labelRect.marginsRemoved(QMarginsF(5, 5, 5, 5)),
                     Qt::AlignVCenter | Qt::AlignLeft, labelStr.c_str(),
                     &labelRect);
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

bool RBoxMarkWidget::inst_valid(const Instance& inst)
{
    return inst.has_x() && inst.has_y() && inst.has_w() && inst.has_h();
}

void RBoxMarkWidget::inst_reset_bbox(ican_mark::Instance& inst)
{
    inst.clear_x();
    inst.clear_y();
    inst.clear_w();
    inst.clear_h();
}

void RBoxMarkWidget::inst_reset(ican_mark::Instance& inst)
{
    inst.clear_degree();
    this->inst_reset_bbox(inst);
}
