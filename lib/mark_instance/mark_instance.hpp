#ifndef __MARK_INSTANCE_HPP__
#define __MARK_INSTANCE_HPP__

#include <stdexcept>
#include <string>
#include <utility>

#include <yaml-cpp/yaml.h>

namespace ican_mark
{
class Instance
{
#define INSTANCE_ATTR_IFACE_IMPL(type, name)                                   \
   protected:                                                                  \
    std::pair<bool, type> name;                                                \
                                                                               \
   public:                                                                     \
    type get_##name() const                                                    \
    {                                                                          \
        if (!this->name.first)                                                 \
        {                                                                      \
            throw std::runtime_error("Instance attribute " #name " not set!"); \
        }                                                                      \
                                                                               \
        return this->name.second;                                              \
    }                                                                          \
                                                                               \
    void set_##name(type arg)                                                  \
    {                                                                          \
        this->name.first = true;                                               \
        this->name.second = arg;                                               \
    }                                                                          \
                                                                               \
    bool has_##name() const { return this->name.first; }                       \
    void clear_##name() { this->name.first = false; }

    INSTANCE_ATTR_IFACE_IMPL(int, label)
    INSTANCE_ATTR_IFACE_IMPL(double, degree)
    INSTANCE_ATTR_IFACE_IMPL(double, x)
    INSTANCE_ATTR_IFACE_IMPL(double, y)
    INSTANCE_ATTR_IFACE_IMPL(double, w)
    INSTANCE_ATTR_IFACE_IMPL(double, h)

    operator std::string() const;
};
}  // namespace ican_mark

/** Conversion functions for instances */
namespace YAML
{
template <>
struct convert<ican_mark::Instance>
{
    static Node encode(const ican_mark::Instance& rhs)
    {
        Node node;

        if (rhs.has_label()) node["label"] = rhs.get_label();
        if (rhs.has_degree()) node["degree"] = rhs.get_degree();
        if (rhs.has_x()) node["x"] = rhs.get_x();
        if (rhs.has_y()) node["y"] = rhs.get_y();
        if (rhs.has_w()) node["w"] = rhs.get_w();
        if (rhs.has_h()) node["h"] = rhs.get_h();

        return node;
    }

    static bool decode(const Node& node, ican_mark::Instance& rhs)
    {
        if (node.IsSequence())
        {
            return false;
        }

        rhs.set_label(node["label"].as<int>());
        rhs.set_degree(node["degree"].as<double>());
        rhs.set_x(node["x"].as<double>());
        rhs.set_y(node["y"].as<double>());
        rhs.set_w(node["w"].as<double>());
        rhs.set_h(node["h"].as<double>());

        return true;
    }
};
}  // namespace YAML

#endif
