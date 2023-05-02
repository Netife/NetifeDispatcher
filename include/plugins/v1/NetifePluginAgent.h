//
// Created by Administrator on 2023/5/1.
//

#ifndef NETIFEDISPATCHER_NETIFEPLUGINAGENT_H
#define NETIFEDISPATCHER_NETIFEPLUGINAGENT_H
#include "NetifePlugins.h"
class NetifePlugins;

class NetifePluginAgent {
public:
    NetifePluginAgent() = default;
    virtual ~NetifePluginAgent()= default;
    virtual NetifePlugins* GetRelativePluginRef(const std::string& pluginsName) = 0;//[危险函数，本函数允许插件直接修改插件实例对象] 得到插件的 Ref
    virtual const NetifePlugins& GetRelativePlugin(const std::string& pluginsName) = 0;// [安全函数，本函数允许插件得到插件实例对象的副本] 得到插件的 Copy
    virtual std::string CarryRelativePluginCommand(const std::string& pluginsName, const std::string& command) = 0;// 执行一个命令，得到返回值
    virtual void LogInfo(const std::string& name, const std::string& content) = 0;
};


#endif //NETIFEDISPATCHER_NETIFEPLUGINAGENT_H
