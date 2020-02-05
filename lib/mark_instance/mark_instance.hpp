#ifndef __MARK_INSTANCE_HPP__
#define __MARK_INSTANCE_HPP__

#include <yaml-cpp/yaml.h>

namespace ical_mark
{
struct Instance
{
    int label = 0;
    double degree = 0;
    double x = 0;
    double y = 0;
    double w = 0;
    double h = 0;

    bool valid() const;
    void reset();
    void reset_degree();
    void reset_bbox();
};
}  // namespace ical_mark

/** Conversion functions for instances */
namespace YAML
{
template <>
struct convert<ical_mark::Instance>
{
    static Node encode(const ical_mark::Instance& rhs)
    {
        Node node;

        node["label"] = rhs.label;
        node["degree"] = rhs.degree;
        node["x"] = rhs.x;
        node["y"] = rhs.y;
        node["w"] = rhs.w;
        node["h"] = rhs.h;

        return node;
    }

    static bool decode(const Node& node, ical_mark::Instance& rhs)
    {
        if (node.IsSequence())
        {
            return false;
        }

        rhs.label = node["label"].as<int>();
        rhs.degree = node["degree"].as<double>();
        rhs.x = node["x"].as<double>();
        rhs.y = node["y"].as<double>();
        rhs.w = node["w"].as<double>();
        rhs.h = node["h"].as<double>();

        return true;
    }
};
}  // namespace YAML

#endif
