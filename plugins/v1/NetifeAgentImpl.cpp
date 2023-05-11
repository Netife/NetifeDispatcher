//
// Created by Administrator on 2023/5/2.
//

#include "NetifeAgentImpl.h"
#include "PluginsDispatcher.h"
#define ELPP_THREAD_SAFE
#include "../../lib/log/easylogging++.h"
#include <optional>
using namespace std;
std::optional<NetifePlugins *> NetifeAgentImpl::GetRelativePluginRef(const string &pluginsName) {
    auto instance = Netife::PluginsDispatcher::Instance()->GetPluginInstance(pluginsName);
    if (instance == nullptr){
        return nullopt;
    }
    if (instance->ExposeRefModule()){
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

std::optional<std::string> NetifeAgentImpl::CarryRelativePluginCommand(const string &pluginsName, const string &command) {
    auto plugin = GetRelativePluginRef(pluginsName);
    if (!plugin.has_value()){
        return nullopt;
    }
    return { plugin.value()->DispatcherCommand(command) };
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
