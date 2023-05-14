//
// Created by Administrator on 2023/5/1.
//

#ifndef NETIFEDISPATCHER_COMMANDDESCRIPTOR_H
#define NETIFEDISPATCHER_COMMANDDESCRIPTOR_H
#include <string>
using namespace std;
namespace Netife {

    class CommandDescriptor {
    public:
        string originCommand;
        string description;
        string pluginName;
        string pluginClsid;
        string className;
        bool isPluginCommand;
    };

} // Netife

#endif //NETIFEDISPATCHER_COMMANDDESCRIPTOR_H
