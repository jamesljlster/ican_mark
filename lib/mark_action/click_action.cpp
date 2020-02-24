#include "mark_action.hpp"

namespace ican_mark
{
void ClickAction::reset()
{
    this->s = static_cast<int>(State::MOVE);
    for (auto it = this->varMap.begin(); it != this->varMap.end(); it++)
    {
        it->second = QPointF();
    }
}

void ClickAction::run(QInputEvent* event)
{
    QEvent::Type eventType = event->type();

    switch (static_cast<State>(this->s))
    {
        case State::MOVE:
            if (eventType == QEvent::MouseMove)
            {
                this->varMap["move"] =
                    static_cast<QMouseEvent*>(event)->localPos();
            }

            if (eventType == QEvent::MouseButtonPress)
            {
                this->varMap["press"] =
                    static_cast<QMouseEvent*>(event)->localPos();
                this->s++;
            }

            break;

        case State::PRESS:
            if (eventType == QEvent::MouseMove)
            {
                this->varMap["move"] =
                    static_cast<QMouseEvent*>(event)->localPos();
            }

            if (eventType == QEvent::MouseButtonRelease)
            {
                this->varMap["release"] =
                    static_cast<QMouseEvent*>(event)->localPos();
                this->s++;
            }

            break;

        case State::RELEASE:
            break;
    }
}

void ClickAction::revert() { this->reset(); }

bool ClickAction::finish() const
{
    return (this->s == static_cast<int>(State::RELEASE));
}

int ClickAction::state() const { return this->s; }

const QPointF& ClickAction::operator[](std::string key) const
{
    auto it = this->varMap.find(key);
    if (it == this->varMap.end())
    {
        std::string errMsg =
            std::string("'") + key +
            std::string("' variable not exist in class 'ClickAction'");
        throw std::invalid_argument(errMsg);
    }

    return it->second;
}

}  // namespace ican_mark
