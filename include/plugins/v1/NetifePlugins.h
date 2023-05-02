//
// Created by Administrator on 2023/5/1.
//

#ifndef NETIFEDISPATCHER_NETIFEPLUGINS_H
#define NETIFEDISPATCHER_NETIFEPLUGINS_H
#include <iostream>
#include "NetifePluginAgent.h"
#include "../../models/NetworkRequest.h"
#include "../../models/NetworkResponse.h"
class NetifePluginAgent;

class NetifePlugins {
protected:
    NetifePluginAgent* agent{};
public:

    NetifePlugins() = default;

    void SetNetifePluginAgent(NetifePluginAgent* pluginAgent){
        agent = pluginAgent;
    }

    virtual ~NetifePlugins() = default;

    virtual std::string GetName() const = 0;

    virtual std::string GetVersion() const = 0;

    virtual std::string DispatcherCommand(std::string commandOriginal) = 0;

    //当插件启用的时候 [顺序不明确]
    virtual bool OnEnable() = 0;

    //当插件关闭的时候 [可能是人为禁用单独的插件]
    virtual bool OnDisable() = 0;

    //当插件全部加载完毕的时候 [此时可以使用依赖插件]
    virtual void OnLoaded() = 0;

    //当软件即将退出的时候 [此时所有插件都即将关闭]
    virtual void OnExiting() = 0;

    //以下是基本定义，选择性覆写

    //当请求进入的时候 [即将发送到前端]
    virtual bool OnRequestSendingOut(NetworkRequest* request){
        return true;
    }

    //当请求返回的时候 [还没进入Dispatcher处理器]
    virtual bool OnResponseBackingIn(NetworkResponse* response){
        return true;
    }

    //当请求发送的时候 [不影响请求的流程，本选项为事件槽，多线程传播]
    virtual void OnRequestTrigger(const NetworkRequest& request){

    }

    //当请求进入时 [不影响请求流程，本选项为事件槽，多线程传播]
    virtual void OnResponseTrigger(const NetworkResponse& response){

    }

    //当准备执行一段脚本的时候 [阻塞流程]
    virtual bool OnUseScript(const string& script, NetworkRequest* request){
        return true;
    }

    //当准备把脚本的执行结果返回给Dispatcher时 [阻塞流程]
    virtual bool OnExitScript(const string& script, NetworkResponse* response){
        return true;
    }
};

#endif //NETIFEDISPATCHER_NETIFEPLUGINS_H