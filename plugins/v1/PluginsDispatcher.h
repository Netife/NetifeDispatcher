//
// Created by Administrator on 2023/5/1.
//

#ifndef NETIFEDISPATCHER_PLUGINSDISPATCHER_H
#define NETIFEDISPATCHER_PLUGINSDISPATCHER_H
#include <iostream>
#include <mutex>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <functional>
#include "CommandDescriptor.h"
#include "HookDescriptor.h"
#include "PluginsDescriptor.h"
#include "../../lib/Poco/ClassLoader.h"
#include "../../lib/Poco/Manifest.h"
#include "../../include/plugins/v1/NetifePlugins.h"
typedef Poco::ClassLoader<NetifePlugins> PluginLoader;
typedef Poco::Manifest<NetifePlugins> PluginManifest;
using Poco::SharedLibrary;

namespace Netife {
    class PluginsDispatcher {
    private:
        PluginsDispatcher() = default;
        static std::atomic<PluginsDispatcher*> instance;
        static std::mutex mutex;
        static std::map<std::string, CommandDescriptor> commandLists;
        static std::multimap<std::string, HookDescriptor> hookLists;
        static std::map<std::string, PluginsDescriptor> pluginDescriptorLists; //插件描述类 string 为插件名
        static std::map<std::string, NetifePlugins*> pluginClassMaps; //插件类实体类 string 为 插件名::插件类名
        static std::map<std::string, SharedLibrary*> pluginSharedLibraries; //插件类实体类 string 为 插件名::插件类名
        static PluginLoader pluginLoader;
        bool CheckRelative(const std::string& pluginName, const std::string& versionDescriptor, const std::string& needPluginName);
    public:
        //单例
        PluginsDispatcher(const PluginsDispatcher&) = delete;
        PluginsDispatcher& operator=(const PluginsDispatcher&) = delete;
        static PluginsDispatcher *Instance();
        bool AutoLoadPlugins();
        void RegisterCommand(string command, string description, string pluginsName, string clsid, string className);
        void RegisterHook(string hookNode, string pluginsName, string clsid, string symbol);
        void RegisterPluginDescriptor(string pluginName, PluginsDescriptor descriptor);
        void RegisterPluginClassMaps(string pluginClassWithClassName, NetifePlugins* netifePlugins);
        void UnRegisterAllPlugins();
        void UnRegisterTargetPluginClass(string pluginClassName);
        void UnRegisterTargetPluginLibrary(string pluginName);
        NetifePlugins* GetPluginInstance(string pluginClassWithClassName);
        optional<string> UseCommand(string command);
        void CarryHookPlugin(string hookNode);
        void RegisterPluginSharedLibraries(string pluginName, SharedLibrary* sharedLibrary);
        void UnRegisterTargetSharedLibrary(string pluginName);
        void ProcessAllPlugins(std::function<void (NetifePlugins*)> const& f);
    };

} // Netife

#endif //NETIFEDISPATCHER_PLUGINSDISPATCHER_H
