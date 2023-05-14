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
#include "../../services/NetifeStorage.h"
#include "../../gRpcModel/NetifeMessage.grpc.pb.h"
#include "../../gRpcServices/NetifeJsRemoteImpl.h"
using namespace std;
using Poco::JSON::Object;
using Poco::JSON::Array;
using Poco::JSON::ParseHandler;
using Poco::JSON::Parser;
using Poco::JSON::Stringifier;
using NetifeMessage::NetifeScriptCommandRequest;
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
    for (const auto& item:*node) {
        resBack.insert(std::pair<string, string>(item.first, item.second.toString()));
    }
    return resBack;
}

std::optional<std::vector<std::string>> NetifeAgentImpl::GetRawTextByUUID(std::string UUID, std::optional<std::string> UUID_Sub) {
    return Netife::NetifeStorage::Instance()->GetRawTextByUUID(UUID, UUID_Sub);
}

std::optional<std::vector<std::string>> NetifeAgentImpl::GetRawTextByDst(std::string DstHost, std::optional<std::string> DstPort) {
    return Netife::NetifeStorage::Instance()->GetRawTextByDst(DstHost, DstPort);
}

std::optional<std::vector<std::string>> NetifeAgentImpl::GetRawTextByPid(std::string Pid, std::optional<std::string> ProcessName) {
    return Netife::NetifeStorage::Instance()->GetRawTextByPid(Pid, ProcessName);
}

std::optional<std::vector<std::string>> NetifeAgentImpl::GetRawTextBySrc(std::string SrcHost, std::optional<std::string> SrcPort) {
    return Netife::NetifeStorage::Instance()->GetRawTextBySrc(SrcHost, SrcPort);
}

void NetifeAgentImpl::AddSettings(std::string key, std::string value) {
    Netife::NetifeStorage::Instance()->AddSettings(NetifeAgentImpl::dllName.value(), NetifeAgentImpl::className.value(), key, value);
}

void NetifeAgentImpl::RemoveSettings(std::string key) {
    Netife::NetifeStorage::Instance()->RemoveSettings(NetifeAgentImpl::dllName.value(), NetifeAgentImpl::className.value(), key);
}

bool NetifeAgentImpl::ExitsSettings(std::string key) {
    return Netife::NetifeStorage::Instance()->ExitsSettings(NetifeAgentImpl::dllName.value(), NetifeAgentImpl::className.value(), key);
}

void NetifeAgentImpl::UpdateSettings(std::string key, std::string value) {
    Netife::NetifeStorage::Instance()->UpdateSettings(NetifeAgentImpl::dllName.value(), NetifeAgentImpl::className.value(), key, value);
}

std::optional<std::string> NetifeAgentImpl::GetSettings(std::string key) {
    return Netife::NetifeStorage::Instance()->GetSettings(NetifeAgentImpl::dllName.value(), NetifeAgentImpl::className.value(), key);
}

std::optional<std::string> NetifeAgentImpl::CalcJs(std::string jsContent, std::map<std::string, std::string> paras) {
    NetifeScriptCommandRequest request;
    request.set_script_name("NetifeInstanceCalc");
    request.set_export_function(jsContent);
    for (auto item:paras) {
        request.mutable_params()->insert({item.first, item.second});
    }
    return Netife::NetifeJsRemoteImpl::instance->ProcessScriptCommand(request);
}
