#include "markarea.h"

#include <cstdio>
#include <iostream>

#include <QKeyEvent>
#include <QMouseEvent>
#include <Qt>

using namespace std;

MarkArea::MarkArea(QWidget* parent) : QWidget(parent)
{
    this->setMouseTracking(true);
    this->setFocus();
}

bool MarkArea::event(QEvent* event)
{
    bool ret = false;
    QEvent::Type eventType = event->type();

    if (eventType == QEvent::MouseMove)
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        this->testAction.run(me);
        ret = true;
    }
    else if (eventType == QEvent::MouseButtonPress ||
             eventType == QEvent::MouseButtonRelease)
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::MouseButton::LeftButton)
        {
            this->testAction.run(me);
            ret = true;
        }
    }
    else if (eventType == QEvent::KeyPress)
    {
        QKeyEvent* ke = static_cast<QKeyEvent*>(event);
        if (ke->key() == Qt::Key_Escape)
        {
            this->testAction.reset();
            ret = true;
        }
    }

    cout << "state: " << this->testAction.state;
    if (this->testAction.finish())
    {
        printf(" (%d, %d) (%d, %d)", this->testAction.ptPress.x(),
               this->testAction.ptPress.y(), this->testAction.ptRelease.x(),
               this->testAction.ptRelease.y());
    }
    cout << endl;

    if (ret)
        return ret;
    else
        return QWidget::event(event);
}
