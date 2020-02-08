#include <iostream>
#include <string>

#include <mark_instance.hpp>

using namespace std;
using namespace ical_mark;

int main()
{
    YAML::Node node;
    Instance inst;

    inst.label = 1;
    inst.degree = 2.2;
    inst.x = 3.3;
    inst.y = 4.4;
    inst.w = 5.5;
    inst.h = 6.6;

    node = inst;
    cout << "Yaml node:" << endl;
    cout << node << endl;
    cout << endl;

    inst = node.as<Instance>();
    cout << "Instance:" << endl;
    cout << string(inst) << endl;
    cout << endl;

    return 0;
}
