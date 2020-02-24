#include "mark_action.hpp"

namespace ican_mark
{
const QPointF operator*(const QPointF& point, const QPointF& factor)
{
    return QPointF(point.x() * factor.x(), point.y() * factor.y());
}

const QPointF operator/(const QPointF& point, const QPointF& divisor)
{
    return QPointF(point.x() / divisor.x(), point.y() / divisor.y());
}
}  // namespace ican_mark
