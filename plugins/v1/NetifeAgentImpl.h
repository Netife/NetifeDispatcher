//
// Created by Administrator on 2023/5/2.
//

#ifndef NETIFEDISPATCHER_NETIFEAGENTIMPL_H
#define NETIFEDISPATCHER_NETIFEAGENTIMPL_H
#include "../../include/plugins/v1/NetifePluginAgent.h"
#include <optional>
#include <utility>
using namespace std;
class NetifeAgentImpl : public NetifePluginAgent{
public:
    explicit NetifeAgentImpl(bool a, optional<std::string> b, optional<std::string> c) :
    NetifePluginAgent(a,std::move(b),std::move(c)){}
    ~NetifeAgentImpl() override;
    std::optional<NetifePlugins*> GetRelativePluginRef(const std::string& pluginsName) override;//[危险函数，本函数允许插件直接修改插件实例对象] 得到插件的 Ref
    std::optional<const NetifePlugins*> GetRelativePlugin(const std::string& pluginsName) override;// [安全函数，本函数允许插件得到插件实例对象的副本] 得到插件的 Copy
    std::optional<std::string> CarryRelativePluginCommand(const std::string& pluginsName, const std::string& command) override;// 执行一个命令，得到返回值
    void LogInfo(const std::string& content) override;
    void LogWarn(const std::string& content) override;
    void LogError(const std::string& content) override;
    void LogDebug(const std::string& content) override;
    bool IsExisted(const std::string& dllName, const std::string &className) override;
};


#endif //NETIFEDISPATCHER_NETIFEAGENTIMPL_H
