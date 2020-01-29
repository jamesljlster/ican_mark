#ifndef __MARK_ACTION_HPP__
#define __MARK_ACTION_HPP__

#include <QInputEvent>

namespace ical_mark
{
class ActionBase
{
   public:
    virtual void reset() = 0;
    virtual void run(QInputEvent* event) = 0;
    virtual bool finish() = 0;
};

}  // namespace ical_mark

#endif
