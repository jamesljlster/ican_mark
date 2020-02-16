#include "mark_action.hpp"

namespace ical_mark
{
void RBoxMark::reset()
{
    this->s = static_cast<int>(State::INIT);
    for (auto it = this->varMap.begin(); it != this->varMap.end(); it++)
    {
        it->second = TwiceClick();
    }
}

void RBoxMark::run(QInputEvent* event)
{
    switch (static_cast<State>(this->s))
    {
        case State::INIT:
            this->varMap["degree"].run(event);
            if (this->varMap["degree"].finish())
            {
                this->s++;
            }

            break;

        case State::DEGREE_FIN:
            this->varMap["bbox"].run(event);
            if (this->varMap["bbox"].finish())
            {
                this->s++;
            }

            break;

        case State::BBOX_FIN:
            break;
    }
}

void RBoxMark::revert()
{
    switch (static_cast<State>(this->s))
    {
        case State::INIT:
            this->varMap["degree"].revert();

            break;

        case State::DEGREE_FIN:
            if (this->varMap["bbox"].state() == 0)
            {
                this->varMap["degree"].revert();
                this->s--;
            }
            else
            {
                this->varMap["bbox"].revert();
            }

            break;

        case State::BBOX_FIN:
            this->varMap["bbox"].revert();
            this->s--;

            break;
    }
}

void RBoxMark::shift(const QPointF& vec)
{
    this->varMap["degree"].shift(vec);
    this->varMap["bbox"].shift(vec);
}

void RBoxMark::scale(double ratio)
{
    this->varMap["degree"].scale(ratio);
    this->varMap["bbox"].scale(ratio);
}

bool RBoxMark::finish() const
{
    return (static_cast<State>(this->s) == State::BBOX_FIN);
}

int RBoxMark::state() const { return this->s; }

const TwiceClick& RBoxMark::operator[](std::string key) const
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

const RBoxMark RBoxMark::operator+(const QPointF& data) const
{
    RBoxMark ret = (*this);
    ret.varMap["degree"] = ret.varMap["degree"] + data;
    ret.varMap["bbox"] = ret.varMap["bbox"] + data;
    return ret;
}

const RBoxMark RBoxMark::operator-(const QPointF& data) const
{
    RBoxMark ret = (*this);
    ret.varMap["degree"] = ret.varMap["degree"] - data;
    ret.varMap["bbox"] = ret.varMap["bbox"] - data;
    return ret;
}

const RBoxMark RBoxMark::operator*(const QPointF& data) const
{
    RBoxMark ret = (*this);
    ret.varMap["degree"] = ret.varMap["degree"] * data;
    ret.varMap["bbox"] = ret.varMap["bbox"] * data;
    return ret;
}

const RBoxMark RBoxMark::operator/(const QPointF& data) const
{
    RBoxMark ret = (*this);
    ret.varMap["degree"] = ret.varMap["degree"] / data;
    ret.varMap["bbox"] = ret.varMap["bbox"] / data;
    return ret;
}

const RBoxMark RBoxMark::operator+(const RBoxMark& data) const
{
    RBoxMark ret = (*this);
    ret.varMap["degree"] = ret.varMap["degree"] + data["degree"];
    ret.varMap["bbox"] = ret.varMap["bbox"] + data["bbox"];
    return ret;
}

const RBoxMark RBoxMark::operator-(const RBoxMark& data) const
{
    RBoxMark ret = (*this);
    ret.varMap["degree"] = ret.varMap["degree"] - data["degree"];
    ret.varMap["bbox"] = ret.varMap["bbox"] - data["bbox"];
    return ret;
}

const RBoxMark RBoxMark::operator*(const RBoxMark& data) const
{
    RBoxMark ret = (*this);
    ret.varMap["degree"] = ret.varMap["degree"] * data["degree"];
    ret.varMap["bbox"] = ret.varMap["bbox"] * data["bbox"];
    return ret;
}

const RBoxMark RBoxMark::operator/(const RBoxMark& data) const
{
    RBoxMark ret = (*this);
    ret.varMap["degree"] = ret.varMap["degree"] / data["degree"];
    ret.varMap["bbox"] = ret.varMap["bbox"] / data["bbox"];
    return ret;
}

}  // namespace ical_mark
