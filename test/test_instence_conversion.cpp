#include <iostream>

#include <mark_instance.hpp>

using namespace std;
using namespace ical_mark;

int main()
{
    Instance inst;

    inst.label = 1;
    inst.degree = 2.2;
    inst.x = 3.3;
    inst.y = 4.4;
    inst.w = 5.5;
    inst.h = 6.6;

    YAML::Node node;
    node = inst;

    cout << "Yaml node:" << endl;
    cout << node << endl;
    cout << endl;

    inst = node.as<Instance>();
    cout << "Instance:" << endl;
    cout << "label: " << inst.label << endl;
    cout << "degree: " << inst.degree << endl;
    cout << "x: " << inst.x << endl;
    cout << "y: " << inst.y << endl;
    cout << "w: " << inst.w << endl;
    cout << "h: " << inst.h << endl;
    cout << endl;

    return 0;
}
