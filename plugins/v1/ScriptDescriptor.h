//
// Created by Administrator on 2023/5/2.
//

#ifndef NETIFEDISPATCHER_SCRIPTDESCRIPTOR_H
#define NETIFEDISPATCHER_SCRIPTDESCRIPTOR_H
#include <string>
#include <vector>

using namespace std;
namespace Netife {

    class ScriptDescriptor {
    public:
        string name;
        string clsid;
        string author;
        string version;
        string description;
        vector<string> exportFunctionName;
    };

} // Netife

#endif //NETIFEDISPATCHER_SCRIPTDESCRIPTOR_H
