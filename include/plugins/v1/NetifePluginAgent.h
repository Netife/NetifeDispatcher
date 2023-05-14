//
// Created by Administrator on 2023/5/1.
//

#ifndef NETIFEDISPATCHER_NETIFEPLUGINAGENT_H
#define NETIFEDISPATCHER_NETIFEPLUGINAGENT_H
#include "NetifePlugins.h"
#include <optional>
#include <filesystem>
#include <utility>
#include <map>
#include <vector>
class NetifePlugins;

class NetifePluginAgent {
protected:
    bool isPluginInstance;
    std::optional<std::string> className;
    std::optional<std::string> dllName;
public:
    explicit NetifePluginAgent(bool a, std::optional<std::string> b, std::optional<std::string> c):isPluginInstance(a),
    dllName(std::move(b)), className(std::move(c)){};
    virtual ~NetifePluginAgent()= default;
    virtual std::optional<NetifePlugins*> GetRelativePluginRef(const std::string& pluginsName) = 0;//[危险函数，本函数允许插件直接修改插件实例对象，但需要插件授权] 得到插件的 Ref
    virtual std::optional<const NetifePlugins*> GetRelativePlugin(const std::string& pluginsName) = 0;// [安全函数，本函数允许插件得到插件实例对象的副本] 得到插件的 Copy
    virtual std::optional<std::string> CarryPluginCommand(const std::string& commandPrefix, const std::string& rawCommand)  = 0;// 执行一个命令，得到返回值
    virtual void LogInfo(const std::string& content) = 0; //Log Information
    virtual void LogWarn(const std::string& content) = 0; //Log Warning
    virtual void LogError(const std::string& content) = 0; //Log Error
    virtual void LogDebug(const std::string& content) = 0; //Log Debug
    virtual bool IsExisted(const std::string& dllName, const std::string &className) = 0; // 判断一个函数是否存在
    virtual std::string GetMainModuleDataPath() = 0; //得到主插件存储空间
    virtual std::string GetPluginDataPath() = 0; //得到插件实体类存储空间
    virtual std::map<std::string, std::string> WrapperCommandResultWithMap(const std::string& res) = 0;
    virtual std::optional<std::string> CarryPluginCommandWithVector(const std::string& commandPrefix, std::vector<std::string> params)  = 0;// 执行一个命令，得到返回值
    virtual std::optional<std::string> CarryPluginCommandWithMap(const std::string& commandPrefix, std::map<std::string, std::optional<std::string>> params)  = 0;// 执行一个命令，得到返回值
    virtual std::optional<std::vector<std::string>> GetRawTextByUUID(std::string UUID, std::optional<std::string> UUID_Sub) = 0;
    virtual std::optional<std::vector<std::string>> GetRawTextByPid(std::string Pid, std::optional<std::string> ProcessName) = 0;
    virtual std::optional<std::vector<std::string>> GetRawTextByDst(std::string DstHost, std::optional<std::string> DstPort) = 0;
    virtual std::optional<std::vector<std::string>> GetRawTextBySrc(std::string SrcHost, std::optional<std::string> SrcPort) = 0;
    virtual void AddSettings(std::string key, std::string value) = 0;
    virtual void UpdateSettings(std::string key, std::string value) = 0;
    virtual void RemoveSettings(std::string key) = 0;
    virtual bool ExitsSettings(std::string key) = 0;
    virtual std::optional<std::string> GetSettings(std::string key) = 0;
    virtual std::optional<std::string> CalcJs(std::string jsContent, std::map<std::string, std::string>paras) = 0;
};


#endif //NETIFEDISPATCHER_NETIFEPLUGINAGENT_H
