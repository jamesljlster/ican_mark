#include "mark_widget.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>

#include <QBrush>
#include <QKeyEvent>
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
    // Call parent reset function
    ImageView::reset(image);

    // Reset marking state
    this->annoList = instList;
    this->markAction.reset();
    this->moveAction.reset();

    // Reset view and image region
    this->imageRegion = this->find_image_region(
        QPointF(this->bgImage.width() / 2, this->bgImage.height() / 2),
        this->size(), this->scaleRatio);
    this->viewRegion =
        this->find_view_region(this->imageRegion.size().toSize(), this->size());

    // Repaint and raise signal
    this->repaint();
    emit stateChanged(this->annoList);
}

void RBoxMarkWidget::set_mark_label(int label) { this->label = label; }
void RBoxMarkWidget::set_hl_instance_index(int index)
{
    this->highlightInst = index;
    this->repaint();
}

void RBoxMarkWidget::set_image_region(const QRectF& imageRegion)
{
    this->imageRegion = imageRegion;
    this->viewRegion =
        this->find_view_region(this->imageRegion.size().toSize(), this->size());
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
    emit stateChanged(this->annoList);
}

bool RBoxMarkWidget::event(QEvent* event)
{
    bool ret = false;
    QEvent::Type eventType = event->type();

    // Run FSM of marking action
    if (eventType == QEvent::MouseMove)
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        this->mousePos = me->pos();
        this->markAction.run(me);
        this->moveAction.run(me);
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

        if (me->button() == Qt::MouseButton::MiddleButton)
        {
            this->moveAction.run(me);
            ret = true;
        }
    }
    else if (eventType == QEvent::KeyPress)
    {
        QKeyEvent* ke = static_cast<QKeyEvent*>(event);
        if (ke->key() == Qt::Key_Escape)
        {
            this->markAction.revert();
            ret = true;
        }
        else if (ke->key() == Qt::Key_Backspace)
        {
            this->markAction.reset();
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
            this->imageRegion = this->find_image_region(
                this->regionPosCache +
                    this->scaling_to_image(this->moveAction["press"] -
                                           this->moveAction["move"]),
                this->size(), this->scaleRatio);
            break;

        case ClickAction::State::RELEASE:
            this->moveAction.reset();
            break;
    }

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
            this->curInst.reset();
            this->markAction.reset();

            // Raise signal
            emit stateChanged(this->annoList);

            break;
    }

    if (ret)
    {
        this->repaint();
        return ret;
    }
    else
    {
        return QWidget::event(event);
    }
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
    this->imageRegion = this->find_image_region(
        this->imageRegion.center(), event->size(), this->scaleRatio);
    this->viewRegion = this->find_view_region(this->imageRegion.size().toSize(),
                                              event->size());
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

void RBoxMarkWidget::draw_aim_crosshair(const QPoint& center, double degree,
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

    QLine line1(  //
        QPoint(center.x() - shift * center.y(), 0),
        QPoint(center.x() + shift * (height - center.y()), height));
    QLine line2(  //
        QPoint(0, center.y() + shift * center.x()),
        QPoint(width, center.y() - shift * (width - center.x())));

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
    painter.drawRect(
        QRectF(this->scaling_to_view(QPointF(-halfWidth, -halfHeight)),
               this->scaling_to_view(QPointF(halfWidth, halfHeight))));
}

void RBoxMarkWidget::draw_anchor(const QPoint& pos, const StyleAnchor& style)
{
    // Setup painter and drawing style
    QPainter painter(this);

    painter.setRenderHint(style.rendHint);
    painter.setCompositionMode(style.compMode);
    painter.setPen(QPen(style.penColor, style.lineWidth));

    // Draw anchor point
    painter.drawEllipse(pos, style.radius, style.radius);
}
