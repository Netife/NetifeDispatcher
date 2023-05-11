//
// Created by Administrator on 2023/5/1.
//

#ifndef NETIFEDISPATCHER_NETIFEPLUGINAGENT_H
#define NETIFEDISPATCHER_NETIFEPLUGINAGENT_H
#include "NetifePlugins.h"
#include <optional>
#include <filesystem>
#include <utility>
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
    virtual std::optional<std::string> CarryRelativePluginCommand(const std::string& pluginsName, const std::string& command) = 0;// 执行一个命令，得到返回值
    virtual void LogInfo(const std::string& content) = 0; //Log Information
    virtual void LogWarn(const std::string& content) = 0; //Log Warning
    virtual void LogError(const std::string& content) = 0; //Log Error
    virtual void LogDebug(const std::string& content) = 0; //Log Debug
    virtual bool IsExisted(const std::string& dllName, const std::string &className) = 0; // 判断一个函数是否存在
};


#endif //NETIFEDISPATCHER_NETIFEPLUGINAGENT_H
