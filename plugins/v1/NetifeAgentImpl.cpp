//
// Created by Administrator on 2023/5/2.
//

#include "NetifeAgentImpl.h"
#include "PluginsDispatcher.h"
#define ELPP_THREAD_SAFE
#include "../../lib/log/easylogging++.h"
#include <optional>
#include <filesystem>
#include "../../lib/Poco/JSON/Array.h"
#include "../../lib/Poco/JSON/Parser.h"
#include "../../lib/Poco/JSON/ParseHandler.h"
#include "../../lib/Poco/JSON/Stringifier.h"
using namespace std;
using Poco::JSON::Object;
using Poco::JSON::Array;
using Poco::JSON::ParseHandler;
using Poco::JSON::Parser;
using Poco::JSON::Stringifier;
namespace var = Poco::Dynamic;
std::optional<NetifePlugins *> NetifeAgentImpl::GetRelativePluginRef(const string &pluginsName) {
    auto instance = Netife::PluginsDispatcher::Instance()->GetPluginInstance(pluginsName);
    if (instance == nullptr){
        return nullopt;
    }
    if (instance->ExposeRefModule() || instance->ExposeRefJudgingByName(NetifeAgentImpl::dllName.value()
    + "::" + NetifeAgentImpl::className.value())){
        return nullopt; // 如果插件不允许暴露自己的引用，那么 Dispatcher 按照约定不给予实例
    }
    return { instance };
}

std::optional<const NetifePlugins*> NetifeAgentImpl::GetRelativePlugin(const string &pluginsName) {
    const NetifePlugins* netifePlugins = Netife::PluginsDispatcher::Instance()->GetPluginInstance(pluginsName);
    if (netifePlugins == nullptr){
        return nullopt;
    }
    return { netifePlugins };
}

void NetifeAgentImpl::LogInfo(const string &content) {
    if (NetifeAgentImpl::isPluginInstance){
        CLOG(INFO, "Plugin") << "[" << NetifeAgentImpl::dllName.value() << "::" << NetifeAgentImpl::className.value() << "] " << content;
    }else{
        CLOG(INFO, "Script") << "[" << NetifeAgentImpl::dllName.value() << "::" << NetifeAgentImpl::className.value() << "] " << content;
    }
}

NetifeAgentImpl::~NetifeAgentImpl() {

}

void NetifeAgentImpl::LogError(const string &content) {
    if (NetifeAgentImpl::isPluginInstance){
        CLOG(ERROR, "Plugin") << "[" << NetifeAgentImpl::dllName.value() << "::" << NetifeAgentImpl::className.value() << "] " << content;
    }else{
        CLOG(ERROR, "Script") << "[" << NetifeAgentImpl::dllName.value() << "::" << NetifeAgentImpl::className.value() << "] " << content;
    }
}

void NetifeAgentImpl::LogWarn(const string &content) {
    if (NetifeAgentImpl::isPluginInstance){
        CLOG(WARNING, "Plugin") << "[" << NetifeAgentImpl::dllName.value() << "::" << NetifeAgentImpl::className.value() << "] " << content;
    }else{
        CLOG(WARNING, "Script") << "[" << NetifeAgentImpl::dllName.value() << "::" << NetifeAgentImpl::className.value() << "] " << content;
    }
}

void NetifeAgentImpl::LogDebug(const string &content) {
    if (NetifeAgentImpl::isPluginInstance){
        CLOG(DEBUG, "Plugin") << "[" << NetifeAgentImpl::dllName.value() << "::" << NetifeAgentImpl::className.value() << "] " << content;
    }else{
        CLOG(DEBUG, "Script") << "[" << NetifeAgentImpl::dllName.value() << "::" << NetifeAgentImpl::className.value() << "] " << content;
    }
}

bool NetifeAgentImpl::IsExisted(const string &dllName, const string &className) {
    auto plugin = GetRelativePluginRef(dllName + "::" + className);
    if (!plugin.has_value()){
        return false;
    }
    return true;
}

std::string NetifeAgentImpl::GetMainModuleDataPath() {
    std::filesystem::path dataPath = "";
    if (NetifeAgentImpl::isPluginInstance){
        dataPath = "plugins";
    }else{
        dataPath = "scripts";
    }
    dataPath = dataPath / "data" / NetifeAgentImpl::dllName.value();
    if (!exists(dataPath)){
        create_directory(dataPath);
    }
    return dataPath.string();
}

std::string NetifeAgentImpl::GetPluginDataPath() {
    std::filesystem::path dataPath = "";
    if (NetifeAgentImpl::isPluginInstance){
        dataPath = "plugins";
    }else{
        dataPath = "scripts";
    }
    dataPath = dataPath / "data" / NetifeAgentImpl::dllName.value();
    if (!exists(dataPath)){
        create_directory(dataPath);
    }
    dataPath = dataPath / NetifeAgentImpl::className.value();
    if (!exists(dataPath)){
        create_directory(dataPath);
    }
    return dataPath.string();
}

std::optional<std::string>
NetifeAgentImpl::CarryPluginCommand(const string &commandPrefix, const string &rawCommand) {
    return { Netife::PluginsDispatcher::Instance()->UseRawCommand(commandPrefix, rawCommand) };
}

std::optional<std::string>
NetifeAgentImpl::CarryPluginCommandWithVector(const string &commandPrefix, std::vector<string> params) {
    return { Netife::PluginsDispatcher::Instance()->UseCommandByVector(commandPrefix, params) };
}

std::optional<std::string> NetifeAgentImpl::CarryPluginCommandWithMap(const string &commandPrefix,
                                                                      std::map<std::string, std::optional<std::string>> params) {
    return { Netife::PluginsDispatcher::Instance()->UseCommandByMap(commandPrefix, params) };
}

std::map<std::string, std::string> NetifeAgentImpl::WrapperCommandResultWithMap(const string &res) {
    std::map<std::string, std::string> resBack;
    Parser parser;
    var::Var result = parser.parse(res); // 解析 JSON 文件
    Object::Ptr node = result.extract<Object::Ptr>(); // 获取 JSON 对象
    for (auto item:*node) {
        resBack.insert(std::pair<string, string>(item.first, item.second.toString()));
    }
    return resBack;
}
