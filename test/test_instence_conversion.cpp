#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <mark_instance.hpp>

using namespace std;
using namespace ical_mark;

int main()
try
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

    vector<Instance> instList;
    instList.push_back(inst);
    instList.push_back(inst);

    node = instList;
    YAML::Emitter out;
    out << node;
    cout << node << endl;
    std::ofstream fWriter("test.mark");
    fWriter << out.c_str();
    fWriter.close();

    node = YAML::LoadFile("test.mark");
    instList = node.as<vector<Instance>>();

    return 0;
}
catch (exception& ex)
{
    cout << endl;
    cout << "Error!" << endl;
    cout << ex.what() << endl;
    cout << endl;
    return -1;
}
