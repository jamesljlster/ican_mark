#ifndef __MARK_ACTION_HPP__
#define __MARK_ACTION_HPP__

#include <exception>
#include <stdexcept>
#include <string>

#include <QInputEvent>
#include <QPointF>

namespace ical_mark
{
template <typename argType>
class ActionBase
{
   public:
    virtual void reset() = 0;
    virtual void run(QInputEvent* event) = 0;
    virtual void revert() = 0;
    virtual void shift(const QPointF& vec) = 0;
    virtual void scale(double ratio) = 0;

    virtual bool finish() const = 0;
    virtual int state() const = 0;

    virtual argType operator[](std::string) const = 0;
};

class ClickAction : public ActionBase<const QPointF&>
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
    void revert();
    void shift(const QPointF& vec);
    void scale(double ratio);

    bool finish() const;
    int state() const;

    const QPointF& operator[](std::string key) const;

   protected:
    int s = static_cast<int>(State::MOVE);
    std::map<std::string, QPointF> varMap = {
        {"move", QPointF()}, {"press", QPointF()}, {"release", QPointF()}};
};

class TwiceClick : public ActionBase<const ClickAction&>
{
   public:
    enum class State
    {
        INIT,
        POS1_FIN,
        POS2_FIN
    };

    void reset();
    void run(QInputEvent* event);
    void revert();
    void shift(const QPointF& vec);
    void scale(double ratio);

    bool finish() const;
    int state() const;

    const ClickAction& operator[](std::string key) const;

   protected:
    int s = static_cast<int>(State::INIT);
    std::map<std::string, ClickAction> varMap = {{"pos1", ClickAction()},
                                                 {"pos2", ClickAction()}};
};

class RBoxMark : public ActionBase<const TwiceClick&>
{
   public:
    enum class State
    {
        INIT,
        DEGREE_FIN,
        BBOX_FIN
    };

    void reset();
    void run(QInputEvent* event);
    void revert();
    void shift(const QPointF& vec);
    void scale(double ratio);

    bool finish() const;
    int state() const;

    const TwiceClick& operator[](std::string key) const;

   protected:
    int s = static_cast<int>(State::INIT);
    std::map<std::string, TwiceClick> varMap = {{"degree", TwiceClick()},
                                                {"bbox", TwiceClick()}};
};

}  // namespace ical_mark

#endif
