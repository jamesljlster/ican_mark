#ifndef __MARK_ACTION_HPP__
#define __MARK_ACTION_HPP__

#include <exception>
#include <stdexcept>
#include <string>

#include <QInputEvent>
#include <QPointF>

namespace ical_mark
{
const QPointF operator*(const QPointF& point, const QPointF& factor);
const QPointF operator/(const QPointF& point, const QPointF& divisor);

#define MARK_ACTION_OPERATOR_IMPL(ClassName, ArgClass, op, accessor)     \
    const ClassName operator op(const ArgClass& data) const              \
    {                                                                    \
        ClassName ret = (*this);                                         \
        for (auto it = ret.varMap.begin(); it != ret.varMap.end(); it++) \
        {                                                                \
            it->second = it->second op accessor;                         \
        }                                                                \
        return ret;                                                      \
    }

#define MARK_ACTION_OPERATOR_QPOINTF(ClassName, op) \
    MARK_ACTION_OPERATOR_IMPL(ClassName, QPointF, op, data)
#define MARK_ACTION_OPERATOR_CLASSNAME(ClassName, op) \
    MARK_ACTION_OPERATOR_IMPL(ClassName, ClassName, op, data[it->first])

#define MARK_ACTION_OPERATOR(ClassName)          \
   public:                                       \
    MARK_ACTION_OPERATOR_QPOINTF(ClassName, +)   \
    MARK_ACTION_OPERATOR_QPOINTF(ClassName, -)   \
    MARK_ACTION_OPERATOR_QPOINTF(ClassName, *)   \
    MARK_ACTION_OPERATOR_QPOINTF(ClassName, /)   \
    MARK_ACTION_OPERATOR_CLASSNAME(ClassName, +) \
    MARK_ACTION_OPERATOR_CLASSNAME(ClassName, -) \
    MARK_ACTION_OPERATOR_CLASSNAME(ClassName, *) \
    MARK_ACTION_OPERATOR_CLASSNAME(ClassName, /)

template <typename argType>
class ActionBase
{
   public:
    virtual void reset() = 0;
    virtual void run(QInputEvent* event) = 0;
    virtual void revert() = 0;

    virtual bool finish() const = 0;
    virtual int state() const = 0;

    virtual const argType& operator[](std::string key) const = 0;

   protected:
    std::map<std::string, argType> varMap;
};

class ClickAction : public ActionBase<QPointF>
{
    MARK_ACTION_OPERATOR(ClickAction)

   public:
    enum class State
    {
        MOVE,
        PRESS,
        RELEASE
    };

    ClickAction()
    {
        this->varMap = {
            {"move", QPointF()}, {"press", QPointF()}, {"release", QPointF()}};
    }

    void reset();
    void run(QInputEvent* event);
    void revert();

    bool finish() const;
    int state() const;

    const QPointF& operator[](std::string key) const;

   protected:
    int s = static_cast<int>(State::MOVE);
};

class TwiceClick : public ActionBase<ClickAction>
{
    MARK_ACTION_OPERATOR(TwiceClick)

   public:
    enum class State
    {
        INIT,
        POS1_FIN,
        POS2_FIN
    };

    TwiceClick()
    {
        this->varMap = {{"pos1", ClickAction()}, {"pos2", ClickAction()}};
    }

    void reset();
    void run(QInputEvent* event);
    void revert();

    bool finish() const;
    int state() const;

    const ClickAction& operator[](std::string key) const;

   protected:
    int s = static_cast<int>(State::INIT);
};

class RBoxMark : public ActionBase<TwiceClick>
{
    MARK_ACTION_OPERATOR(RBoxMark)

   public:
    enum class State
    {
        INIT,
        DEGREE_FIN,
        BBOX_FIN
    };

    RBoxMark()
    {
        this->varMap = {{"degree", TwiceClick()}, {"bbox", TwiceClick()}};
    }

    void reset();
    void run(QInputEvent* event);
    void revert();

    bool finish() const;
    int state() const;

    const TwiceClick& operator[](std::string key) const;

   protected:
    int s = static_cast<int>(State::INIT);
};

}  // namespace ical_mark

#endif
