//
// Created by Administrator on 2023/5/1.
//

#ifndef NETIFEDISPATCHER_PLUGINSDESCRIPTOR_H
#define NETIFEDISPATCHER_PLUGINSDESCRIPTOR_H
#include <string>
#include <vector>
using namespace std;
namespace Netife {
    class PluginsDescriptor {
    public:
        string name;
        string clsid;
        string version;
        string exportWay;
        string description;
        string exportCommand;
        vector<string> exportClassName;
    };

} // Netife

#endif //NETIFEDISPATCHER_PLUGINSDESCRIPTOR_H
