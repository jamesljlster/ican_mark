#include "mark_instance.hpp"

using namespace std;
using namespace ical_mark;

bool Instance::valid() const { return (this->w > 0 && this->h > 0); }

void Instance::reset()
{
    this->label = 0;
    this->reset_degree();
    this->reset_bbox();
}

void Instance::reset_degree() { this->degree = 0; }
void Instance::reset_bbox()
{
    this->x = 0;
    this->y = 0;
    this->w = 0;
    this->h = 0;
}

Instance::operator string() const
{
    YAML::Node node;
    YAML::Emitter out;

    node = *this;
    node.SetStyle(YAML::EmitterStyle::Flow);
    out << node;

    return out.c_str();
}
