//
// Created by Administrator on 2023/5/1.
//

#ifndef NETIFEDISPATCHER_HOOKDESCRIPTOR_H
#define NETIFEDISPATCHER_HOOKDESCRIPTOR_H
#include <string>
using namespace std;
namespace Netife {

    class HookDescriptor {
    public:
        string hookNode;
        string pluginName;
        string pluginClsid;
        string pluginSymbol;
    };

} // Netife

#endif //NETIFEDISPATCHER_HOOKDESCRIPTOR_H
