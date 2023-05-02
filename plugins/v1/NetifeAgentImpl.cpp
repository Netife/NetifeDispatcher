//
// Created by Administrator on 2023/5/2.
//

#include "NetifeAgentImpl.h"
#include "PluginsDispatcher.h"
#include "../../lib/log/easylogging++.h"
#include <optional>
using namespace std;
std::optional<NetifePlugins *> NetifeAgentImpl::GetRelativePluginRef(const string &pluginsName) {
    auto instance = Netife::PluginsDispatcher::Instance()->GetPluginInstance(pluginsName);
    if (instance == nullptr){
        return nullopt;
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

void NetifeAgentImpl::LogInfo(const string &name, const string &content) {
    CLOG(INFO, "Plugin") << "[" << name << "] " << content;
}

NetifeAgentImpl::~NetifeAgentImpl() {

}
