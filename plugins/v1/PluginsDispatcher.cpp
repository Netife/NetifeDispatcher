//
// Created by Administrator on 2023/5/1.
//
#include <iostream>
#include "PluginsDispatcher.h"
#include <filesystem>
#include "../../lib/log/easylogging++.h"
#include "../../lib/Poco/JSON/JSON.h"
#include "../../lib/Poco/JSON/Array.h"
#include "../../lib/Poco/JSON/Parser.h"
#include "../../lib/Poco/JSON/ParseHandler.h"
#include "../../lib/Poco/JSON/Stringifier.h"
#include "../../utils/TextHelper.h"
#include "../../lib/Poco/ClassLoader.h"
#include "../../lib/Poco/Manifest.h"
#include "../../include/plugins/v1/NetifePlugins.h"
#include "PluginsDescriptor.h"
#include "NetifeAgentImpl.h"
#define PLUGINS_DISPATCHER_VERSION "v1"

using namespace std;
using namespace Netife;
using Poco::JSON::Object;
using Poco::JSON::Array;
using Poco::JSON::ParseHandler;
using Poco::JSON::Parser;
using Poco::JSON::Stringifier;

namespace fs = std::filesystem;
namespace var = Poco::Dynamic;
typedef void (*BaseHookNode)(NetifePluginAgent* agent);

namespace Netife {
    // C++ 就是依托答辩，这个定义太傻了
    std::mutex PluginsDispatcher::mutex;
    std::atomic<PluginsDispatcher *> PluginsDispatcher::instance;
    std::map<std::string, CommandDescriptor> PluginsDispatcher::commandLists;
    std::multimap<std::string, HookDescriptor> PluginsDispatcher::hookLists;
    std::map<std::string, PluginsDescriptor> PluginsDispatcher::pluginDescriptorLists; //插件描述类 string 为插件名
    std::map<std::string, NetifePlugins*> PluginsDispatcher::pluginClassMaps; //插件类实体类 string 为 插件名::插件类名
    PluginLoader PluginsDispatcher::pluginLoader;
    std::map<std::string, SharedLibrary*> PluginsDispatcher::pluginSharedLibraries;
    PluginsDispatcher *PluginsDispatcher::Instance() {
        // 双 Check 锁
        if (instance == nullptr) {
            std::lock_guard<std::mutex> lock(mutex);
            if (instance == nullptr) {
                instance = new PluginsDispatcher();
            }
        }
        return instance;
    }

    bool PluginsDispatcher::AutoLoadPlugins() {

        // 扫描插件目录，以便于挂载插件

        fs::path basicPath = "plugins";
        bool isWholeOk = true;
        for (auto &plugin: fs::directory_iterator(basicPath / "bin")) // 遍历目录
        {

            auto filePath = basicPath / "config" / (plugin.path().stem().string() + ".json");
            if (!fs::exists(filePath)) {
                CLOG(ERROR, "PluginsDispatcher") << "WARN FOR NO CONFIG DESCRIPTION IN CONFIG DIR. "
                                                    "PLEASE CHECK \"" << plugin.path().filename()
                                                 << "\" CONFIG OR REINSTALL IT.";
                continue;
            }


            ifstream jsonFile(filePath);
            Parser parser;
            var::Var result = parser.parse(jsonFile); // 解析 JSON 文件
            Object::Ptr node = result.extract<Object::Ptr>(); // 获取 JSON 对象
            if (PLUGINS_DISPATCHER_VERSION != node->get("coreRelative").toString()) {
                CLOG(WARNING, "PluginsDispatcher") << "THE PLUGINS DISPATCHER "
                                                      "VERSION IS NOT FULFILLED WITH THIS CORE. IT MAY CAUSE SOME PROBLEMS";
            }
            PluginsDescriptor pluginsDescriptor;
            pluginsDescriptor.name = node->get("name").toString();
            pluginsDescriptor.clsid = node->get("clsid").toString();
            pluginsDescriptor.version = node->get("version").toString();
            pluginsDescriptor.exportWay = node->get("exportWay").toString();
            pluginsDescriptor.description = node->get("description").toString();
            pluginsDescriptor.exportCommand = node->get("exportCommand").toString();
            pluginsDescriptor.exportClassName = TextHelper::split(node->get("exportClassName").toString(), "|");

            //检查依赖关系

            bool isOk = true;
            Array::Ptr relativeChainsArr = node->getArray("relativeChains");
            for (int i = 0; i < relativeChainsArr->size(); ++i) {
                Object::Ptr obj = relativeChainsArr->getObject(i);
                if(!CheckRelative(obj->get("name").toString(), obj->get("version").toString(), pluginsDescriptor.name)){
                    isOk = false;
                    isWholeOk = false;
                }
            }

            if(!isOk){
                continue;
            }
            //注册自由命令

            Array::Ptr registerCommandArr = node->getArray("registerCommand");
            for (int i = 0; i < registerCommandArr->size(); ++i) {
                Object::Ptr obj = registerCommandArr->getObject(i);
                RegisterCommand(obj->get("command").toString(), obj->get("description").toString(),
                                pluginsDescriptor.name, pluginsDescriptor.clsid,
                                obj->get("class").toString());
            }

            //注册Hook回调地址

            Array::Ptr registerHookArr = node->getArray("registerHook");
            for (int i = 0; i < registerHookArr->size(); ++i) {
                Object::Ptr obj = registerHookArr->getObject(i);
                RegisterHook(obj->get("node").toString(),
                             pluginsDescriptor.name, pluginsDescriptor.clsid, obj->get("symbol").toString());
            }

            //保存插件本体

            RegisterPluginDescriptor(pluginsDescriptor.name, pluginsDescriptor);

            //加载动态函数

            SharedLibrary* library = new SharedLibrary((basicPath / "bin" / plugin.path().filename()).string());
            RegisterPluginSharedLibraries(pluginsDescriptor.name, library);

            //加载动态类

            pluginLoader.loadLibrary((basicPath / "bin" / plugin.path().filename()).string());
            PluginLoader::Iterator it(pluginLoader.begin());
            PluginLoader::Iterator end(pluginLoader.end());
            for (; it != end; ++it) {
                PluginManifest::Iterator itMan(it->second->begin());
                PluginManifest::Iterator endMan(it->second->end());
                for (; itMan != endMan; ++itMan) {
                    CLOG(INFO, "PluginsDispatcher") << "Load plugin class "
                                                    << itMan->name() << " in plugin library \""
                                                    << pluginsDescriptor.name << "\"";
                    NetifePlugins *netifePlugin = pluginLoader.create(itMan->name());
                    RegisterPluginClassMaps(pluginsDescriptor.name + "::" + itMan->name(),
                                            netifePlugin);
                    NetifePluginAgent* agent = new NetifeAgentImpl();
                    netifePlugin->SetNetifePluginAgent(agent);
                    CLOG(INFO, "PluginsDispatcher") << "[" << pluginsDescriptor.name << "]["
                                                    << itMan->name() << "] Name:\"" << netifePlugin->GetName() << "\""
                                                    << " Version\"" << netifePlugin->GetVersion() << "\"";
                    netifePlugin->OnEnable();
                    CarryHookPlugin("netife.network.receive"); //Hook插件启动
                }
            }
        }
        return isWholeOk;
    }

    bool PluginsDispatcher::CheckRelative(const std::string &pluginName,
                                          const std::string &versionDescriptor,
                                          const std::string &needPluginName) {
        fs::path filePath = "plugins";
        filePath = filePath / "config" / (pluginName + ".json");
        if (!fs::exists(filePath)) {
            CLOG(ERROR, "PluginsDispatcher")
                    << "Plugins Relative \"" << needPluginName << "\"--->\"" << pluginName
                    << "\" do not exist.";
            return false;
        }
        ifstream jsonFile(filePath);
        Parser parser;
        var::Var result = parser.parse(jsonFile);
        Object::Ptr node = result.extract<Object::Ptr>();
        string version = node->get("version");
        auto versionLimit = TextHelper::split(versionDescriptor, "|");
        for (auto sp: versionLimit) {
            // 依赖要求大于这个版本号
            if (sp[0] == '^') {
                auto require = TextHelper::split(sp.substr(1, sp.length() - 1), ".");
                auto really = TextHelper::split(version, ".");
                for (int i = 0; i < 2; ++i) { //版本号要求是: A.B.C
                    if (stoi(require[i]) <= stoi(really[i])) {
                        CLOG(ERROR, "PluginsDispatcher")
                                << "Plugins Relative \"" << needPluginName << "\"--->\""
                                << pluginName
                                << "\" do not fulfill the requirement. The requirement is greater than "
                                << sp.substr(1, sp.length() - 1)
                                << " while the really version is " << version;
                        return false;
                    }
                }
            }

            //依赖要求小于这个版本号
            if (sp[0] == '<') {
                auto require = TextHelper::split(sp.substr(1, sp.length() - 1), ".");
                auto really = TextHelper::split(version, ".");
                for (int i = 0; i < 2; ++i) { //版本号要求是: A.B.C
                    if (stoi(require[i]) >= stoi(really[i])) {
                        CLOG(ERROR, "PluginsDispatcher")
                                << "Plugins Relative \"" << needPluginName << "\"--->\""
                                << pluginName
                                << "\" do not fulfill the requirement. The requirement is less than "
                                << sp.substr(1, sp.length() - 1)
                                << " while the really version is " << version;
                        return false;
                    }
                }
            }

            //依赖要求等于这个版本号
            if (sp[0] == '=') {
                auto require = TextHelper::split(sp.substr(1, sp.length() - 1), ".");
                auto really = TextHelper::split(version, ".");
                for (int i = 0; i < 2; ++i) { //版本号要求是: A.B.C
                    if (stoi(require[i]) != stoi(really[i])) {
                        CLOG(ERROR, "PluginsDispatcher")
                                << "Plugins Relative \"" << needPluginName << "\"--->\""
                                << pluginName
                                << "\" do not fulfill the requirement. The requirement equals to "
                                << sp.substr(1, sp.length() - 1)
                                << " while the really version is " << version;
                        return false;
                    }
                }
            }
        }
        return true;
    }

    void PluginsDispatcher::RegisterCommand(string command, string description, string pluginsName, string clsid,
                                            string className) {
        CommandDescriptor commandDescriptor;
        commandDescriptor.pluginClsid = clsid;
        commandDescriptor.originCommand = command;
        commandDescriptor.pluginName = pluginsName;
        commandDescriptor.description = description;
        commandDescriptor.className = className;
        commandLists.insert(
                std::pair<string, CommandDescriptor>(TextHelper::split(command, " ")[0], commandDescriptor));
    }

    void PluginsDispatcher::RegisterHook(string hookNode, string pluginsName, string clsid,
                                         string symbol) {
        HookDescriptor hookDescriptor;
        hookDescriptor.hookNode = hookNode;
        hookDescriptor.pluginName = pluginsName;
        hookDescriptor.pluginClsid = clsid;
        hookDescriptor.pluginSymbol = symbol;
        hookLists.insert(std::pair<string, HookDescriptor>(hookNode, hookDescriptor));
    }

    void PluginsDispatcher::RegisterPluginDescriptor(string pluginName, PluginsDescriptor descriptor) {
        pluginDescriptorLists.insert(std::pair<string, PluginsDescriptor>(pluginName, descriptor));
    }

    void PluginsDispatcher::RegisterPluginClassMaps(string pluginClassWithClassName, NetifePlugins *netifePlugins) {
        pluginClassMaps.insert(std::pair<string, NetifePlugins *>(pluginClassWithClassName, netifePlugins));
    }

    void PluginsDispatcher::UnRegisterTargetPluginClass(string pluginClassName) {
        delete pluginClassMaps[pluginClassName];
        //@MAYBE-BUG 可能会二次清除同一份地址
        pluginClassMaps.erase(pluginClassName);
    }

    void PluginsDispatcher::UnRegisterTargetPluginLibrary(string pluginName) {
        fs::path basicPath = "plugins";
        pluginLoader.unloadLibrary((basicPath / "bin" / (pluginName + ".dll")).string());
    }

    void PluginsDispatcher::UnRegisterAllPlugins() {
        for (auto plugin:pluginClassMaps) {
            delete plugin.second;
        }
        for (auto libs:pluginDescriptorLists) {
            UnRegisterTargetPluginLibrary(libs.first);
        }
    }

    NetifePlugins* PluginsDispatcher::GetPluginInstance(string pluginClassWithClassName) {
        return pluginClassMaps[pluginClassWithClassName];
    }

    optional<string> PluginsDispatcher::UseCommand(string command) {
        auto iter = commandLists.find(command);
        if(iter == commandLists.end()){
            return nullopt;
        }
        string plugin = iter->second.pluginName + "::" + iter->second.className;
        string res = pluginClassMaps[plugin]->DispatcherCommand(command);
        return { res };
    }

    void PluginsDispatcher::CarryHookPlugin(string hookNode) {
        auto iter = hookLists.find(hookNode);
        if(iter == hookLists.end()){
            return;
        }
        NetifePluginAgent* agent = new NetifeAgentImpl();
        for (int i = 0, len = hookLists.count(hookNode);i < len; ++i,++iter) {
            string plugin = iter->second.pluginName;
            BaseHookNode func = (BaseHookNode)pluginSharedLibraries[plugin]->getSymbol(iter->second.pluginSymbol);
            func(agent);
        }
        delete agent;
    }

    void PluginsDispatcher::RegisterPluginSharedLibraries(string pluginName, SharedLibrary* sharedLibrary) {
        pluginSharedLibraries.insert(std::pair<string,SharedLibrary*>(pluginName, sharedLibrary));
    }

    void PluginsDispatcher::UnRegisterTargetSharedLibrary(string pluginName) {
        pluginSharedLibraries[pluginName]->unload();
        pluginSharedLibraries.erase(pluginName);
    }
} // Netife