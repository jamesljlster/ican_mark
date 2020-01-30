#ifndef __MARK_ACTION_HPP__
#define __MARK_ACTION_HPP__

#include <exception>
#include <stdexcept>
#include <string>

#include <QInputEvent>

namespace ical_mark
{
template <typename argType>
class ActionBase
{
   public:
    virtual void reset() = 0;
    virtual void run(QInputEvent* event) = 0;

    virtual bool finish() const = 0;
    virtual int state() const = 0;

    virtual argType operator[](std::string) const = 0;
};

class ClickAction : public ical_mark::ActionBase<const QPoint&>
{
   public:
    enum class State
    {
        MOVE,
        PRESS,
        RELEASE
    };

    void reset()
    {
        this->s = static_cast<int>(State::MOVE);
        for (auto it = this->varMap.begin(); it != this->varMap.end(); it++)
        {
            it->second = QPoint();
        }
    }

    void run(QInputEvent* event)
    {
        QEvent::Type eventType = event->type();

        switch ((State)this->s)
        {
            case State::MOVE:
                if (eventType == QEvent::MouseMove)
                {
                    this->varMap["move"] =
                        static_cast<QMouseEvent*>(event)->pos();
                }

                if (eventType == QEvent::MouseButtonPress)
                {
                    this->varMap["press"] =
                        static_cast<QMouseEvent*>(event)->pos();
                    this->s++;
                }

                break;

            case State::PRESS:
                if (eventType == QEvent::MouseMove)
                {
                    this->varMap["move"] =
                        static_cast<QMouseEvent*>(event)->pos();
                }

                if (eventType == QEvent::MouseButtonRelease)
                {
                    this->varMap["release"] =
                        static_cast<QMouseEvent*>(event)->pos();
                    this->s++;
                }

                break;

            case State::RELEASE:
                break;
        }
    }

    bool finish() const
    {
        return (this->s == static_cast<int>(State::RELEASE));
    }

    int state() const { return this->s; }

    const QPoint& operator[](std::string key) const
    {
        auto it = this->varMap.find(key);
        if (it == this->varMap.end())
        {
            std::string errMsg =
                std::string("'") + key +
                std::string("' variable not exist in class '") +
                typeid(*this).name() + std::string("'");
            throw std::invalid_argument(errMsg);
        }

        return it->second;
    }

   protected:
    int s = static_cast<int>(State::MOVE);
    std::map<std::string, QPoint> varMap = {
        {"move", QPoint()}, {"press", QPoint()}, {"release", QPoint()}};
};

}  // namespace ical_mark

#endif
