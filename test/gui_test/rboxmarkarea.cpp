#include "markarea.h"

#include <cmath>
#include <cstdio>
#include <iostream>

#include <QBrush>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QRectF>
#include <QSize>
#include <Qt>
#include <QtMath>

using namespace std;
using namespace ical_mark;

RBoxMarkArea::RBoxMarkArea(QWidget* parent) : QWidget(parent)
{
    this->setMouseTracking(true);
    this->setFocus();

    this->bgImage = QImage("color_map.png");
    // this->setCursor(Qt::BlankCursor);
}

bool RBoxMarkArea::event(QEvent* event)
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
    cout << "state: " << this->markAction.state();
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

            cout << "-" << this->markAction["degree"].state();
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

            cout << "-" << this->markAction["bbox"].state();
            break;

        case RBoxMark::State::BBOX_FIN:

            // Append instance to annotation list
            this->annoList.push_back(this->curInst);
            this->curInst.reset();
            this->markAction.reset();

            break;
    }

    cout << " degree: " << this->curInst.degree;
    cout << " x: " << this->curInst.x;
    cout << " y: " << this->curInst.y;
    cout << " w: " << this->curInst.w;
    cout << " h: " << this->curInst.h;

    if (this->markAction.finish())
    {
        cout << " Finished";
    }

    cout << endl;

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

void RBoxMarkArea::paintEvent(QPaintEvent* paintEvent)
{
    int width = this->width();
    int height = this->height();

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
    this->draw_aim_crosshair(this->mousePos, this->curInst.degree);

    // Draw marked instances
    for (auto anno : this->annoList)
    {
        this->draw_rotated_bbox(anno);
    }

    // Draw rbox marking progress
    if (static_cast<RBoxMark::State>(this->markAction.state()) ==
        RBoxMark::State::DEGREE_FIN)
    {
        if (this->curInst.valid())
        {
            this->draw_rotated_bbox(this->curInst);
        }
    }
}

double RBoxMarkArea::find_distance(const QPointF& p1, const QPointF& p2)
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

double RBoxMarkArea::find_degree(const QPoint& from, const QPoint& to)
{
    qreal theta = qAtan2(from.y() - to.y(), to.x() - from.x());
    theta = qRadiansToDegrees(theta) - 90.0;
    if (theta < -180.0)
    {
        theta += 360.0;
    }

    return theta;
}

QRectF RBoxMarkArea::find_bbox(const QPoint& pos1, const QPoint& pos2,
                               double degree)
{
    Instance inst;
    inst.degree = degree;
    this->fill_bbox(inst, pos1, pos2);
    return QRectF(inst.x, inst.y, inst.w, inst.h);
}

void RBoxMarkArea::fill_bbox(Instance& inst, const QPoint& pos1,
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

void RBoxMarkArea::draw_aim_crosshair(const QPoint& center, double degree,
                                      const QColor& penColor)
{
    int width = this->width();
    int height = this->height();

    // Setup painter and drawing style
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    painter.setPen(penColor);

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

void RBoxMarkArea::draw_rotated_bbox(const Instance& inst, int ctrRad,
                                     const QColor& penColor)
{
    double xScale =
        (double)this->markSize.width() / (double)this->bgImage.width();
    double yScale =
        (double)this->markSize.height() / (double)this->bgImage.height();

    // Setup painter and drawing style
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    painter.setPen(penColor);

    // Draw center point
    QPointF center = this->markBase + QPointF(inst.x * xScale, inst.y * yScale);
    painter.drawEllipse(center, ctrRad, ctrRad);

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

void RBoxMarkArea::draw_rotated_bbox(const QRectF& bbox, double degree,
                                     int ctrRad, const QColor& penColor)
{
    Instance inst;
    inst.degree = degree;
    inst.x = bbox.x();
    inst.y = bbox.y();
    inst.w = bbox.width();
    inst.h = bbox.height();

    this->draw_rotated_bbox(inst, ctrRad, penColor);
}
