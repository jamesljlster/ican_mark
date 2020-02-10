#include "markwidget.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>

#include <QBrush>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QSize>
#include <Qt>
#include <QtMath>

using namespace std;
using namespace ical_mark;

RBoxMarkWidget::RBoxMarkWidget(QWidget* parent) : QWidget(parent)
{
    this->setMouseTracking(true);
    this->setCursor(Qt::BlankCursor);

    // Set default painting style
    this->style.rboxHL.lineWidth = 2;
    this->style.rboxHL.centerRad = 3;
}

void RBoxMarkWidget::reset(const QImage& image,
                           const std::vector<ical_mark::Instance>& instList)
{
    this->bgImage = image;
    this->annoList = instList;
    this->repaint();

    emit stateChanged(this->annoList);
}

void RBoxMarkWidget::set_mark_label(int label) { this->label = label; }
void RBoxMarkWidget::set_hl_instance_index(int index)
{
    this->highlightInst = index;
    this->repaint();
}

const vector<Instance>& RBoxMarkWidget::annotation_list()
{
    return this->annoList;
}

void RBoxMarkWidget::delete_instances(const std::vector<size_t>& indList)
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
    switch (static_cast<RBoxMark::State>(this->markAction.state()))
    {
        case RBoxMark::State::INIT:

            if (static_cast<TwiceClick::State>(
                    this->markAction["degree"].state()) ==
                TwiceClick::State::POS1_FIN)
            {
                // Calculate and set degree
                this->curInst.degree = this->find_degree(
                    this->markAction["degree"]["pos1"]["release"],
                    this->mousePos);
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
                this->fill_bbox(this->curInst,
                                this->markAction["bbox"]["pos1"]["release"],
                                this->mousePos);
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
        this->markSize =
            this->bgImage.size().scaled(this->size(), Qt::KeepAspectRatio);

        int markWidth = this->markSize.width();
        int markHeight = this->markSize.height();

        this->markBase =
            QPoint((width - markWidth) / 2, (height - markHeight) / 2);

        QRectF srcRect(0, 0, this->bgImage.width(), this->bgImage.height());
        QRectF bgRect(this->markBase.x(), this->markBase.y(), markWidth,
                      markHeight);

        painter.drawImage(bgRect, this->bgImage, srcRect);
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

double RBoxMarkWidget::find_degree(const QPoint& from, const QPoint& to)
{
    qreal theta = qAtan2(from.y() - to.y(), to.x() - from.x());
    theta = qRadiansToDegrees(theta) - 90.0;
    if (theta < -180.0)
    {
        theta += 360.0;
    }

    return theta;
}

void RBoxMarkWidget::fill_bbox(Instance& inst, const QPoint& pos1,
                               const QPoint& pos2)
{
    double x, y, w, h;
    double degree = inst.degree;
    double xScale =
        (double)this->markSize.width() / (double)this->bgImage.width();
    double yScale =
        (double)this->markSize.height() / (double)this->bgImage.height();

    QLineF line1, line2;
    QPointF pos1f(pos1), pos2f(pos2);
    QPointF crossPt1, crossPt2, center;

    // Set center point of lines
    line1.setP1(pos1f);
    line2.setP1(pos2f);

    // Find cross point 1 and height
    line1.setAngle(degree);
    line2.setAngle(degree + 90);

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    line1.intersect(line2, &crossPt1);
#else
    line1.intersects(line2, &crossPt1);
#endif
    h = this->find_distance(pos2f, crossPt1) / xScale;

    // Find cross point 2 and width
    line1.setAngle(degree + 90);
    line2.setAngle(degree);

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    line1.intersect(line2, &crossPt2);
#else
    line1.intersects(line2, &crossPt2);
#endif
    w = this->find_distance(pos2f, crossPt2) / yScale;

    // Find center point
    line1.setPoints(pos1f, pos2f);
    line2.setPoints(crossPt1, crossPt2);
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    line1.intersect(line2, &center);
#else
    line1.intersects(line2, &center);
#endif

    center = center - this->markBase;
    x = center.x() / xScale;
    y = center.y() / yScale;

    inst.x = x;
    inst.y = y;
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
    double xScale =
        (double)this->markSize.width() / (double)this->bgImage.width();
    double yScale =
        (double)this->markSize.height() / (double)this->bgImage.height();

    // Setup painter and drawing style
    QPainter painter(this);

    painter.setRenderHint(style.rendHint);
    painter.setCompositionMode(style.compMode);
    painter.setPen(QPen(style.penColor, style.lineWidth));

    // Draw center point
    QPointF center = this->markBase + QPointF(inst.x * xScale, inst.y * yScale);
    if (style.centerRad > 0)
    {
        painter.drawEllipse(center, style.centerRad, style.centerRad);
    }

    // Draw rotated bounding box
    double halfWidth = inst.w * xScale / 2;
    double halfHeight = inst.h * yScale / 2;

    QPointF topLeft = QPointF(-halfWidth, -halfHeight);
    QPointF bottomRight = QPointF(halfWidth, halfHeight);

    QTransform transform;
    transform.translate(center.x(), center.y());
    transform.rotate(-inst.degree);
    painter.setTransform(transform);
    painter.drawRect(QRectF(topLeft, bottomRight));
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
