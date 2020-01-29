#include <iostream>

#include <QPoint>

#include <mark_action.hpp>

using namespace std;
using namespace ical_mark;

class TestAction : public ActionBase
{
   protected:
    enum class FSM
    {
        INIT,
        MOVE,
        PRESS,
        RELEASE,
        FINISH
    };

   public:
    int main(int argc, char* argv[])
    {
        this->reset();
        return 0;
    }

    void reset() { this->state = (int)FSM::INIT; }
    void run(QInputEvent* event)
    {
        QEvent::Type eventType = event->type();

        switch ((FSM)this->state)
        {
            case FSM::INIT:
                this->state++;
                break;

            case FSM::MOVE:
                if (eventType == QEvent::MouseMove)
                {
                    this->ptMove = ((QMouseEvent*)event)->pos();
                }

                if (eventType == QEvent::MouseButtonPress)
                {
                    this->ptPress = ((QMouseEvent*)event)->pos();
                    this->state++;
                }

                break;

            case FSM::PRESS:
                if (eventType == QEvent::MouseMove)
                {
                    this->ptMove = ((QMouseEvent*)event)->pos();
                }

                if (eventType == QEvent::MouseButtonRelease)
                {
                    this->ptRelease = ((QMouseEvent*)event)->pos();
                    this->state++;
                }

                break;

            case FSM::RELEASE:
                this->state++;
                break;

            case FSM::FINISH:
                break;
        }
    }

    bool finish() { return (this->state == (int)FSM::FINISH); }

   protected:
    int state = (int)FSM::INIT;
    QPoint ptMove;
    QPoint ptPress;
    QPoint ptRelease;
};

int main(int argc, char* argv[])
{
    TestAction test;
    return test.main(argc, argv);
}
