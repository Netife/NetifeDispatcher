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
#include <regex>
#include <utility>
#include "PluginsDescriptor.h"
#include "ScriptDescriptor.h"
#include "NetifeAgentImpl.h"
#include "../../services/NetifeStorage.h"
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

typedef void (*BaseHookNode)(NetifePluginAgent *agent);

namespace Netife {
    // C++ 就是依托答辩，这个定义太傻了
    std::mutex PluginsDispatcher::mutex;
    std::atomic<PluginsDispatcher *> PluginsDispatcher::instance;
    std::map<std::string, CommandDescriptor> PluginsDispatcher::commandLists;
    std::multimap<std::string, HookDescriptor> PluginsDispatcher::hookLists;
    std::map<std::string, PluginsDescriptor> PluginsDispatcher::pluginDescriptorLists; //插件描述类 string 为插件名
    std::map<std::string, NetifePlugins *> PluginsDispatcher::pluginClassMaps; //插件类实体类 string 为 插件名::插件类名
    PluginLoader PluginsDispatcher::pluginLoader;
    std::map<std::string, SharedLibrary *> PluginsDispatcher::pluginSharedLibraries;
    std::map<std::string, ScriptDescriptor> PluginsDispatcher::scriptDescriptorLists;
    std::map<std::string, std::string> PluginsDispatcher::scriptMaps;

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
            pluginsDescriptor.author = node->get("author").toString();
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
                if (!CheckRelative(obj->get("name").toString(), obj->get("version").toString(),
                                   pluginsDescriptor.name)) {
                    isOk = false;
                    isWholeOk = false;
                }
            }

            if (!isOk) {
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

            SharedLibrary *library = new SharedLibrary((basicPath / "bin" / plugin.path().filename()).string());
            RegisterPluginSharedLibraries(pluginsDescriptor.name, library);

            //加载动态类

            pluginLoader.loadLibrary((basicPath / "bin" / plugin.path().filename()).string());
        }

        PluginLoader::Iterator it(pluginLoader.begin());
        PluginLoader::Iterator end(pluginLoader.end());
        for (; it != end; ++it) {
            PluginManifest::Iterator itMan(it->second->begin());
            PluginManifest::Iterator endMan(it->second->end());
            for (; itMan != endMan; ++itMan) {
                vector<std::string> pathTemp = TextHelper::split(it->first, "\\");
                std::string dllName = pathTemp[2].substr(0, pathTemp[2].length() - 4);
                CLOG(INFO, "PluginsDispatcher") << "Load plugin class "
                                                << itMan->name() << " in plugin library \""
                                                << it->first << "\"";
                NetifePlugins *netifePlugin = pluginLoader.create(itMan->name());
                RegisterPluginClassMaps(dllName + "::" + itMan->name(),
                                        netifePlugin);
                //TODO agent 指针维护
                NetifePluginAgent *agent = new NetifeAgentImpl(true, dllName, itMan->name());
                netifePlugin->SetNetifePluginAgent(agent);
                CLOG(INFO, "PluginsDispatcher") << "[" << dllName << "]["
                                                << itMan->name() << "] Name:\"" << netifePlugin->GetName() << "\""
                                                << " Version\"" << netifePlugin->GetVersion() << "\"";
                if (!netifePlugin->OnEnable()) {
                    CLOG(WARNING, "PluginsDispatcher") << "[" << dllName << "]["
                                                       << itMan->name() << "] Plugin actively refuse to load.";
                    //@NETIFE RULE 一个插件的主动拒绝不会导致框架寄掉，但是框架会抛出提示
                    CLOG(WARNING, "PluginsDispatcher")
                            << "In Netife Rules a plugin actively refuse will not prevent "
                               "the rest plugin load and framework running. However, it maybe cause "
                               "some trouble like relation and command error.";
                    UnRegisterTargetPluginLibrary(dllName + "::" + itMan->name());
                }

                NetifeStorage::Instance()->BuildDataTable(dllName, itMan->name());
            }
        }

        CarryHookPlugin("netife.network.receive"); //Hook插件启动

        for (auto plugin: pluginClassMaps) {
            plugin.second->OnLoaded();
        }
        return isWholeOk;
    }

    bool PluginsDispatcher::CheckRelative(const std::string &pluginName,
                                          const std::string &versionDescriptor,
                                          const std::string &needPluginName,
                                          const std::string &filepathStart) {
        fs::path filePath = filepathStart;
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
            if (sp[0] == '>') {
                auto require = TextHelper::split(sp.substr(1, sp.length() - 1), ".");
                auto really = TextHelper::split(version, ".");
                for (int i = 0; i < 2; ++i) { //版本号要求是: A.B.C
                    if (stoi(require[i]) <= stoi(really[i])) {
                        CLOG(ERROR, "PluginsDispatcher")
                                << filepathStart << " Relative \"" << needPluginName << "\"--->\""
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
                                << filepathStart << " Relative \"" << needPluginName << "\"--->\""
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
                                << filepathStart << " Relative \"" << needPluginName << "\"--->\""
                                << pluginName
                                << "\" do not fulfill the requirement. The requirement equals to "
                                << sp.substr(1, sp.length() - 1)
                                << " while the really version is " << version;
                        return false;
                    }
                }
            }

            // 依赖要求大于等于这个版本号
            if (sp[0] == '^') {
                auto require = TextHelper::split(sp.substr(1, sp.length() - 1), ".");
                auto really = TextHelper::split(version, ".");
                for (int i = 0; i < 2; ++i) { //版本号要求是: A.B.C
                    if (stoi(require[i]) < stoi(really[i])) {
                        CLOG(ERROR, "PluginsDispatcher")
                                << filepathStart << " Relative \"" << needPluginName << "\"--->\""
                                << pluginName
                                << "\" do not fulfill the requirement. The requirement is greater than or equal to "
                                << sp.substr(1, sp.length() - 1)
                                << " while the really version is " << version;
                        return false;
                    }
                }
            }

            //依赖要求小于这个版本号
            if (sp[0] == '~') {
                auto require = TextHelper::split(sp.substr(1, sp.length() - 1), ".");
                auto really = TextHelper::split(version, ".");
                for (int i = 0; i < 2; ++i) { //版本号要求是: A.B.C
                    if (stoi(require[i]) > stoi(really[i])) {
                        CLOG(ERROR, "PluginsDispatcher")
                                << filepathStart << " Relative \"" << needPluginName << "\"--->\""
                                << pluginName
                                << "\" do not fulfill the requirement. The requirement is less than or equal to "
                                << sp.substr(1, sp.length() - 1)
                                << " while the really version is " << version;
                        return false;
                    }
                }
            }
        }
        return true;
    }

    void PluginsDispatcher::RegisterCommand(const string &command, string description, string pluginsName, string clsid,
                                            string className) {
        CommandDescriptor commandDescriptor;
        commandDescriptor.pluginClsid = std::move(clsid);
        commandDescriptor.originCommand = command;
        commandDescriptor.pluginName = std::move(pluginsName);
        commandDescriptor.description = std::move(description);
        commandDescriptor.className = std::move(className);
        commandLists.insert(
                std::pair<string, CommandDescriptor>(TextHelper::split(command, " ")[0], commandDescriptor));
    }

    void PluginsDispatcher::RegisterHook(const string &hookNode, string pluginsName, string clsid,
                                         string symbol) {
        HookDescriptor hookDescriptor;
        hookDescriptor.hookNode = hookNode;
        hookDescriptor.pluginName = std::move(pluginsName);
        hookDescriptor.pluginClsid = std::move(clsid);
        hookDescriptor.pluginSymbol = std::move(symbol);
        hookLists.insert(std::pair<string, HookDescriptor>(hookNode, hookDescriptor));
    }

    void PluginsDispatcher::RegisterPluginDescriptor(const string &pluginName, const PluginsDescriptor &descriptor) {
        pluginDescriptorLists.insert(std::pair<string, PluginsDescriptor>(pluginName, descriptor));
    }

    void
    PluginsDispatcher::RegisterPluginClassMaps(const string &pluginClassWithClassName, NetifePlugins *netifePlugins) {
        pluginClassMaps.insert(std::pair<string, NetifePlugins *>(pluginClassWithClassName, netifePlugins));
    }

    void PluginsDispatcher::UnRegisterTargetPluginClass(const string &pluginClassName) {
        if (!pluginClassMaps[pluginClassName]->OnDisable()) {
            CLOG(ERROR, "PluginsDispatcher")
                    << "Plugin " << pluginClassName << "cause erro when disabled !";
        }
        delete pluginClassMaps[pluginClassName];
        //@MAYBE-BUG 可能会二次清除同一份地址
        pluginClassMaps.erase(pluginClassName);
    }

    void PluginsDispatcher::UnRegisterTargetPluginLibrary(const string &pluginName) {
        fs::path basicPath = "plugins";
        pluginLoader.unloadLibrary((basicPath / "bin" / (pluginName + ".dll")).string());
    }

    void PluginsDispatcher::UnRegisterAllPlugins() {
        for (auto plugin: pluginClassMaps) {
            plugin.second->OnExiting();
            delete plugin.second;
        }
        for (auto libs: pluginDescriptorLists) {
            UnRegisterTargetPluginLibrary(libs.first);
        }
    }

    NetifePlugins *PluginsDispatcher::GetPluginInstance(const string &pluginClassWithClassName) {
        return pluginClassMaps[pluginClassWithClassName];
    }

    optional<string> PluginsDispatcher::UseRawCommand(const string &commandPrefix, const string &rawCommand) {
        auto iter = commandLists.find(commandPrefix);
        if (iter == commandLists.end()) {
            return nullopt;
        }
        string plugin = iter->second.pluginName + "::" + iter->second.className;
        vector<string> realParams = TextHelper::splitByBlankWithSkipBlank(rawCommand);
        return UseCommandByVector(commandPrefix, realParams);
    }

    void PluginsDispatcher::CarryHookPlugin(const string &hookNode) {
        auto iter = hookLists.find(hookNode);
        if (iter == hookLists.end()) {
            return;
        }
        NetifePluginAgent *agent = new NetifeAgentImpl(false, iter->second.pluginName, iter->second.pluginSymbol);
        for (int i = 0, len = hookLists.count(hookNode); i < len; ++i, ++iter) {
            string plugin = iter->second.pluginName;
            BaseHookNode func = (BaseHookNode) pluginSharedLibraries[plugin]->getSymbol(iter->second.pluginSymbol);
            func(agent);
        }
        delete agent;
    }

    void PluginsDispatcher::RegisterPluginSharedLibraries(const string &pluginName, SharedLibrary *sharedLibrary) {
        pluginSharedLibraries.insert(std::pair<string, SharedLibrary *>(pluginName, sharedLibrary));
    }

    void PluginsDispatcher::UnRegisterTargetSharedLibrary(const string &pluginName) {
        pluginSharedLibraries[pluginName]->unload();
        pluginSharedLibraries.erase(pluginName);
    }

    void PluginsDispatcher::ProcessAllPlugins(std::function<void(NetifePlugins *)> const &f) {
        for (auto plugin: pluginClassMaps) {
            f(plugin.second);
        }
    }

    bool PluginsDispatcher::AutoLoadScripts() {
        fs::path basicPath = "scripts";
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

            ScriptDescriptor scriptDescriptor;
            scriptDescriptor.name = node->get("name").toString();
            scriptDescriptor.clsid = node->get("clsid").toString();
            scriptDescriptor.author = node->get("author").toString();
            scriptDescriptor.version = node->get("version").toString();
            scriptDescriptor.description = node->get("description").toString();
            RegisterScriptDescriptor(scriptDescriptor.name, scriptDescriptor);
            //检查依赖关系

            bool isOk = true;
            Array::Ptr relativeChainsArr = node->getArray("relativeChains");
            for (int i = 0; i < relativeChainsArr->size(); ++i) {
                Object::Ptr obj = relativeChainsArr->getObject(i);
                if (!CheckRelative(obj->get("name").toString(), obj->get("version").toString(),
                                   scriptDescriptor.name, "scripts")) {
                    isOk = false;
                    isWholeOk = false;
                }
            }

            if (!isOk) {
                continue;
            }

            //注册监听指令

            Array::Ptr hookUrls = node->getArray("hookUrls");
            for (int i = 0; i < hookUrls->size(); ++i) {
                Object::Ptr obj = hookUrls->getObject(i);
                RegisterScriptFunction(obj->get("regex").toString(),
                                       scriptDescriptor.name + "::" + obj->get("exportFunctionName").toString());
                CLOG(INFO, "PluginsDispatcher") << "Load script " << scriptDescriptor.name + "::" +
                                                                        obj->get("exportFunctionName").toString();
            }
        }
        return isWholeOk;
    }

    void
    PluginsDispatcher::RegisterScriptDescriptor(const std::string &name, const ScriptDescriptor &scriptDescriptor) {
        scriptDescriptorLists.insert(std::pair<string, ScriptDescriptor>(name, scriptDescriptor));
    }

    void PluginsDispatcher::RegisterScriptFunction(const std::string &regex, const std::string &name) {
        scriptMaps.insert(std::pair<string, string>(name, regex));
    }

    void PluginsDispatcher::ProcessMatchScripts(const string &state, const function<void(std::string name)> &f) {
        for (const auto &item: scriptMaps) {
            std::regex reg(item.second);
            std::string beMatched =
                    TextHelper::getBetween(state, "Host:", "\r\n") + "/" + TextHelper::getBetween(state, "/", "HTTP");
            if (std::regex_match(beMatched, reg)) {
                auto temp = TextHelper::split(item.first, "::");
                f(";" + temp[0] + ";" + temp[1].substr(1, temp[1].length()));
            }
        }
    }

    PluginsDispatcher::~PluginsDispatcher() {
        UnRegisterAllPlugins();
    }

    optional<string> PluginsDispatcher::UseCommandByVector(const string &commandPrefix, vector<string> commandParams) {
        auto iter = commandLists.find(commandPrefix);
        if (iter == commandLists.end()) {
            return nullopt;
        }
        string plugin = iter->second.pluginName + "::" + iter->second.className;
        vector<string> beParams = TextHelper::split(iter->second.originCommand, " ");
        map<string, optional<string>> params;
        for (int i = 1; i < commandParams.size(); ++i) { //命令头
            if (i > beParams.size() - 1)
                return nullopt;
            params.insert(std::pair<string, optional<string>>(beParams[i].substr(1, beParams[i].length() - 2),
                                                              commandParams[i]));
        }

        for (int i = commandParams.size() - 1; i < beParams.size() - 1; ++i) {
            //例如 实际上传入了 2 个参数，但是要求有3个参数。那么就是 int i = 2 < 3 再执行一次
            if (beParams[i][0] != '<') {
                return nullopt; //不是 < 那就不是可选的，那么就说明出错了。
            }
            std::string name = beParams[i].substr(1, beParams[i].length() - 2);
            params.insert(std::pair<string, optional<string>>(name, nullopt)); //插入空值
        }
        if (pluginClassMaps[plugin] == nullptr) {
            return nullopt;
        }
        string res = pluginClassMaps[plugin]->DispatcherCommand(commandPrefix, params);
        return {res};
    }

    optional<string>
    PluginsDispatcher::UseCommandByMap(const string &commandPrefix, map<string, optional<string>> commandParams) {
        auto iter = commandLists.find(commandPrefix);
        if (iter == commandLists.end()) {
            return nullopt;
        }
        string plugin = iter->second.pluginName + "::" + iter->second.className;
        string res = pluginClassMaps[plugin]->DispatcherCommand(commandPrefix, commandParams);
        return {res};
    }
} // Netife