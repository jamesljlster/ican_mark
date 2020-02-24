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

#define __attr_encode(name) \
    if (rhs.has_##name()) node[#name] = rhs.get_##name()

        __attr_encode(label);
        __attr_encode(degree);
        __attr_encode(x);
        __attr_encode(y);
        __attr_encode(w);
        __attr_encode(h);

        return node;
    }

    static bool decode(const Node& node, ican_mark::Instance& rhs)
    {
        if (node.IsSequence())
        {
            return false;
        }

#define __attr_decode(type, name) \
    if (node[#name]) rhs.set_##name(node[#name].as<type>())

        __attr_decode(int, label);
        __attr_decode(double, degree);
        __attr_decode(double, x);
        __attr_decode(double, y);
        __attr_decode(double, w);
        __attr_decode(double, h);

        return true;
    }
};
}  // namespace YAML

#endif
