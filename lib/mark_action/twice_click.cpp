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

void TwiceClick::shift(const QPointF& vec)
{
    this->varMap["pos1"].shift(vec);
    this->varMap["pos2"].shift(vec);
}

void TwiceClick::scale(double ratio)
{
    this->varMap["pos1"].scale(ratio);
    this->varMap["pos2"].scale(ratio);
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

const TwiceClick TwiceClick::operator+(const QPointF& data) const
{
    TwiceClick ret = (*this);
    ret.varMap["pos1"] = ret.varMap["pos1"] + data;
    ret.varMap["pos2"] = ret.varMap["pos2"] + data;
    return ret;
}

const TwiceClick TwiceClick::operator-(const QPointF& data) const
{
    TwiceClick ret = (*this);
    ret.varMap["pos1"] = ret.varMap["pos1"] - data;
    ret.varMap["pos2"] = ret.varMap["pos2"] - data;
    return ret;
}

const TwiceClick TwiceClick::operator*(const QPointF& data) const
{
    TwiceClick ret = (*this);
    ret.varMap["pos1"] = ret.varMap["pos1"] * data;
    ret.varMap["pos2"] = ret.varMap["pos2"] * data;
    return ret;
}

const TwiceClick TwiceClick::operator/(const QPointF& data) const
{
    TwiceClick ret = (*this);
    ret.varMap["pos1"] = ret.varMap["pos1"] / data;
    ret.varMap["pos2"] = ret.varMap["pos2"] / data;
    return ret;
}

const TwiceClick TwiceClick::operator+(const TwiceClick& data) const
{
    TwiceClick ret = (*this);
    ret.varMap["pos1"] = ret.varMap["pos1"] + data["pos1"];
    ret.varMap["pos2"] = ret.varMap["pos2"] + data["pos2"];
    return ret;
}

const TwiceClick TwiceClick::operator-(const TwiceClick& data) const
{
    TwiceClick ret = (*this);
    ret.varMap["pos1"] = ret.varMap["pos1"] - data["pos1"];
    ret.varMap["pos2"] = ret.varMap["pos2"] - data["pos2"];
    return ret;
}

const TwiceClick TwiceClick::operator*(const TwiceClick& data) const
{
    TwiceClick ret = (*this);
    ret.varMap["pos1"] = ret.varMap["pos1"] * data["pos1"];
    ret.varMap["pos2"] = ret.varMap["pos2"] * data["pos2"];
    return ret;
}

const TwiceClick TwiceClick::operator/(const TwiceClick& data) const
{
    TwiceClick ret = (*this);
    ret.varMap["pos1"] = ret.varMap["pos1"] / data["pos1"];
    ret.varMap["pos2"] = ret.varMap["pos2"] / data["pos2"];
    return ret;
}

}  // namespace ical_mark
