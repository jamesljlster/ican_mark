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

    void reset();
    void run(QInputEvent* event);

    bool finish() const;
    int state() const;

    const QPoint& operator[](std::string key) const;

   protected:
    int s = static_cast<int>(State::MOVE);
    std::map<std::string, QPoint> varMap = {
        {"move", QPoint()}, {"press", QPoint()}, {"release", QPoint()}};
};

}  // namespace ical_mark

#endif
