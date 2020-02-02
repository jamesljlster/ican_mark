#include "mark_action.hpp"

namespace ical_mark
{
void TwiceClick::reset()
{
    this->s = static_cast<int>(State::INIT);
    for (auto it = this->varMap.begin(); it != this->varMap.end(); it++)
    {
        it->second = ClickAction();
    }
}

void TwiceClick::run(QInputEvent* event)
{
    switch (static_cast<State>(this->s))
    {
        case State::INIT:
            this->varMap["pos1"].run(event);
            if (this->varMap["pos1"].finish())
            {
                this->s++;
            }

            break;

        case State::POS1_FIN:
            this->varMap["pos2"].run(event);
            if (this->varMap["pos2"].finish())
            {
                this->s++;
            }

            break;

        case State::POS2_FIN:
            break;
    }
}

void TwiceClick::revert()
{
    switch (static_cast<State>(this->s))
    {
        case State::INIT:
            break;

        case State::POS1_FIN:
            this->varMap["pos1"].reset();
            this->s--;
            break;

        case State::POS2_FIN:
            this->varMap["pos2"].reset();
            this->s--;
            break;
    }
}

bool TwiceClick::finish() const
{
    return (static_cast<State>(this->s) == State::POS2_FIN);
}

int TwiceClick::state() const { return this->s; }

const ClickAction& TwiceClick::operator[](std::string key) const
{
    auto it = this->varMap.find(key);
    if (it == this->varMap.end())
    {
        std::string errMsg = std::string("'") + key +
                             std::string("' variable not exist in class '") +
                             typeid(*this).name() + std::string("'");
        throw std::invalid_argument(errMsg);
    }

    return it->second;
}

}  // namespace ical_mark
