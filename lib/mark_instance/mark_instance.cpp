#include "mark_instance.hpp"

using namespace std;

namespace ican_mark
{
Instance::operator string() const
{
    YAML::Node node;
    YAML::Emitter out;

    node = *this;
    node.SetStyle(YAML::EmitterStyle::Flow);
    out << node;

    return out.c_str();
}

}  // namespace ican_mark
