//
// Created by Administrator on 2023/5/2.
//

#include "NetifeAgentImpl.h"
#include "PluginsDispatcher.h"
#include "../../lib/log/easylogging++.h"
NetifePlugins *NetifeAgentImpl::GetRelativePluginRef(const string &pluginsName) {
    return Netife::PluginsDispatcher::Instance()->GetPluginInstance(pluginsName);
}

const NetifePlugins& NetifeAgentImpl::GetRelativePlugin(const string &pluginsName) {
    const NetifePlugins* netifePlugins = Netife::PluginsDispatcher::Instance()->GetPluginInstance(pluginsName);
    return *netifePlugins;
}

std::string NetifeAgentImpl::CarryRelativePluginCommand(const string &pluginsName, const string &command) {
    auto plugin = GetRelativePluginRef(pluginsName);
    return plugin->DispatcherCommand(command);
}

void NetifeAgentImpl::LogInfo(const string &name, const string &content) {
    CLOG(INFO, "Plugin") << "[" << name << "] " << content;
}

NetifeAgentImpl::~NetifeAgentImpl() {

}
