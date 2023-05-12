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
#include "ScriptDescriptor.h"
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
        static std::map<std::string, ScriptDescriptor> scriptDescriptorLists;
        static std::map<std::string, std::string> scriptMaps;
        static PluginLoader pluginLoader;
        bool CheckRelative(const std::string& pluginName, const std::string& versionDescriptor, const std::string& needPluginName,
                           const std::string &filepathStart = "plugins");
    public:
        //单例
        PluginsDispatcher(const PluginsDispatcher&) = delete;
        PluginsDispatcher& operator=(const PluginsDispatcher&) = delete;
        ~PluginsDispatcher();
        static PluginsDispatcher *Instance();
        bool AutoLoadPlugins();
        bool AutoLoadScripts();
        void RegisterCommand(const string& command, string description, string pluginsName, string clsid, string className);
        void RegisterHook(const string& hookNode, string pluginsName, string clsid, string symbol);
        void RegisterPluginDescriptor(const string& pluginName, const PluginsDescriptor& descriptor);
        void RegisterPluginClassMaps(const string& pluginClassWithClassName, NetifePlugins* netifePlugins);
        void UnRegisterAllPlugins();
        void UnRegisterTargetPluginClass(const string& pluginClassName);
        void UnRegisterTargetPluginLibrary(const string& pluginName);
        NetifePlugins* GetPluginInstance(const string& pluginClassWithClassName);
        optional<string> UseRawCommand(const string& commandPrefix, const string& rawCommand);
        optional<string> UseCommandByVector(const string& commandPrefix, vector<string> commandParams);
        optional<string> UseCommandByMap(const string& commandPrefix, map<string, optional<string>> commandParams);
        void CarryHookPlugin(const string& hookNode);
        void RegisterPluginSharedLibraries(const string& pluginName, SharedLibrary* sharedLibrary);
        void UnRegisterTargetSharedLibrary(const string& pluginName);
        void ProcessAllPlugins(std::function<void (NetifePlugins*)> const& f);
        void RegisterScriptDescriptor(const std::string& name, const ScriptDescriptor&);
        void RegisterScriptFunction(const std::string& regex, const std::string& name);
        void ProcessMatchScripts(const std::string&, std::function<void (std::string)> const& f);
    };

} // Netife

#endif //NETIFEDISPATCHER_PLUGINSDISPATCHER_H
