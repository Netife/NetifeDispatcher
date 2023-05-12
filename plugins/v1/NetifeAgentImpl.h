//
// Created by Administrator on 2023/5/2.
//

#ifndef NETIFEDISPATCHER_NETIFEAGENTIMPL_H
#define NETIFEDISPATCHER_NETIFEAGENTIMPL_H
#include "../../include/plugins/v1/NetifePluginAgent.h"
#include <optional>
#include <utility>
#include <map>
using namespace std;
class NetifeAgentImpl : public NetifePluginAgent{
public:
    explicit NetifeAgentImpl(bool a, optional<std::string> b, optional<std::string> c) :
    NetifePluginAgent(a,std::move(b),std::move(c)){}
    ~NetifeAgentImpl() override;
    std::optional<NetifePlugins*> GetRelativePluginRef(const std::string& pluginsName) override;//[危险函数，本函数允许插件直接修改插件实例对象] 得到插件的 Ref
    std::optional<const NetifePlugins*> GetRelativePlugin(const std::string& pluginsName) override;// [安全函数，本函数允许插件得到插件实例对象的副本] 得到插件的 Copy
    std::optional<std::string> CarryPluginCommand(const std::string& commandPrefix, const std::string& rawCommand) override;// 执行一个命令，得到返回值
    void LogInfo(const std::string& content) override;
    void LogWarn(const std::string& content) override;
    void LogError(const std::string& content) override;
    void LogDebug(const std::string& content) override;
    bool IsExisted(const std::string& dllName, const std::string &className) override;
    std::string GetMainModuleDataPath() override;
    std::string GetPluginDataPath() override;
    std::map<std::string, std::string> WrapperCommandResultWithMap(const std::string& res) override;
    std::optional<std::string> CarryPluginCommandWithVector(const std::string& commandPrefix, std::vector
            <string> params) override;// 执行一个命令，得到返回值
    std::optional<std::string> CarryPluginCommandWithMap(const std::string& commandPrefix, std::map<std::string, std::optional<std::string>> params) override;// 执行一个命令，得到返回值
    std::optional<std::vector<std::string>> GetRawTextByUUID(std::string UUID, std::optional<std::string> UUID_Sub);
    std::optional<std::vector<std::string>> GetRawTextByPid(std::string Pid, std::optional<std::string> ProcessName);
    std::optional<std::vector<std::string>> GetRawTextByDst(std::string DstHost, std::optional<std::string> DstPort);
    std::optional<std::vector<std::string>> GetRawTextBySrc(std::string SrcHost, std::optional<std::string> SrcPort);
    void AddSettings(std::string key, std::string value) override;
    void UpdateSettings(std::string key, std::string value) override;
    void RemoveSettings(std::string key) override;
    bool ExitsSettings(std::string key) override;
    std::optional<std::string> GetSettings(std::string key) override;
};


#endif //NETIFEDISPATCHER_NETIFEAGENTIMPL_H
