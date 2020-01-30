#ifndef MARKAREA_H
#define MARKAREA_H

#include <QEvent>
#include <QObject>
#include <QWidget>

#include <mark_action.hpp>

class TestAction : public ical_mark::ActionBase
{
   protected:
    enum class FSM
    {
        MOVE,
        PRESS,
        RELEASE
    };

   public:
    void reset() { this->state = (int)FSM::MOVE; }
    void run(QInputEvent* event)
    {
        QEvent::Type eventType = event->type();

        switch ((FSM)this->state)
        {
            case FSM::MOVE:
                if (eventType == QEvent::MouseMove)
                {
                    this->ptMove = static_cast<QMouseEvent*>(event)->pos();
                }

                if (eventType == QEvent::MouseButtonPress)
                {
                    this->ptPress = static_cast<QMouseEvent*>(event)->pos();
                    this->state++;
                }

                break;

            case FSM::PRESS:
                if (eventType == QEvent::MouseMove)
                {
                    this->ptMove = static_cast<QMouseEvent*>(event)->pos();
                }

                if (eventType == QEvent::MouseButtonRelease)
                {
                    this->ptRelease = static_cast<QMouseEvent*>(event)->pos();
                    this->state++;
                }

                break;

            case FSM::RELEASE:
                break;
        }
    }

    bool finish() { return (this->state == (int)FSM::RELEASE); }

    int state = (int)FSM::MOVE;
    QPoint ptMove;
    QPoint ptPress;
    QPoint ptRelease;
};

class MarkArea : public QWidget
{
    Q_OBJECT
   public:
    explicit MarkArea(QWidget* parent = nullptr);

   signals:

   protected:
    bool event(QEvent* event);

    TestAction testAction;
};

#endif  // MARKAREA_H
