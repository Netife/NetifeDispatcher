//
// Created by Administrator on 2023/4/30.
//

#include "NetifeServiceImpl.h"
#define ELPP_THREAD_SAFE
#include "../plugins/v1/PluginsDispatcher.h"
#include "../lib/log/easylogging++.h"
namespace Netife {
    Status NetifeServiceImpl::ProcessProbe(ServerContext *context, const NetifeProbeRequest *request,
                                           NetifeProbeResponse *response) {
        //逻辑：App/Browser -> Probe -> 插件进入事件[决定是否进入] -> 插件捕获事件[触发] -> 脚本捕获事件[自定义修改]
        // -> 交付前端修改 -> 插件回调[决定是否进入] -> 插件捕获事件[触发] -> Probe -> Remote Server

        // 请求进入
        //此处为发送给前端
        bool isNormal = true;
        NetifeProbeRequest scriptRequest = *request;
        Netife::PluginsDispatcher::Instance()->ProcessMatchScripts(
                request->raw_text(),[&](std::string name){
                    //插件事件 [进入脚本时]

                    //[事件] 进入脚本前
                    auto pluginsIntercept = TransNetifeProbeRequest(const_cast<const NetifeProbeRequest*>(&scriptRequest));

                    Netife::PluginsDispatcher::Instance()->ProcessAllPlugins
                            ([&](NetifePlugins* plugins)
                            { isNormal &= plugins->OnUseScript(
                                    scriptRequest.uuid() + name, &pluginsIntercept); });
                    TransNetWorkRequest(pluginsIntercept, &scriptRequest);

                    if (!isNormal){
                        CLOG(WARNING,"NetifeService") << "The request was cancelled by plugin before script load: " << pluginsIntercept.UUID;
                    }

                    scriptRequest.set_uuid(scriptRequest.uuid() + name);
                    auto response = jsRemote.ProcessScript(scriptRequest);
                    //修改一次，然后下次再传
                    scriptRequest.set_dst_ip_addr(response->dst_ip_addr());
                    scriptRequest.set_dst_ip_port(response->dst_ip_port());
                    scriptRequest.set_raw_text(response->response_text());

                    //[事件] 离开脚本后

                    pluginsIntercept = TransNetifeProbeRequest(const_cast<const NetifeProbeRequest*>(&scriptRequest));

                    Netife::PluginsDispatcher::Instance()->ProcessAllPlugins
                            ([&](NetifePlugins* plugins)
                             { isNormal &= plugins->OnExitScript(
                                     scriptRequest.uuid() + name, &pluginsIntercept); });
                    TransNetWorkRequest(pluginsIntercept, &scriptRequest);

                    if (!isNormal){
                        CLOG(WARNING,"NetifeService") << "The request was cancelled by plugin before script load: " << pluginsIntercept.UUID;
                    }
                }
        );
        //最后得到新的request

        if (!isNormal){
            return Status::CANCELLED;
        }

        bool canInto = true;

        const NetifeProbeRequest* scriptRequestConst = &scriptRequest;

        NetworkRequest networkRequest = TransNetifeProbeRequest(scriptRequestConst);

        //此处为事件捕获

        Netife::PluginsDispatcher::Instance()->ProcessAllPlugins
            ([&](NetifePlugins* plugins){ canInto &= plugins->OnRequestSendingOut(&networkRequest); });
        if (!canInto){
            CLOG(WARNING,"NetifeService") << "The request was cancelled by plugin in plugin event: " << networkRequest.UUID;
            return Status::CANCELLED;
        }

        //此处为事件触发
        //TODO 此处改成多线程调用

        Netife::PluginsDispatcher::Instance()->ProcessAllPlugins
                ([networkRequest](NetifePlugins* plugins){ plugins->OnRequestTrigger(networkRequest); });

        NetifeProbeRequest netifeProbeRequest;
        TransNetWorkRequest(networkRequest, &netifeProbeRequest);
        auto netifeResponse = client.UploadRequest(netifeProbeRequest);
        if (!netifeResponse.has_value()){
            CLOG(WARNING,"NetifeService") << "The response was cancelled by frontend: " << networkRequest.UUID;
            return Status::CANCELLED;
        }

        NetworkResponse networkResponse = TransNetifeProbeResponse(&(netifeResponse.value()));

        //此处为回调事件捕获

        Netife::PluginsDispatcher::Instance()->ProcessAllPlugins
            ([&](NetifePlugins* plugins){ canInto &=  plugins->OnResponseBackingIn(&networkResponse); });


        if (!canInto){
            CLOG(WARNING,"NetifeService") << "The response was cancelled by plugin in plugin slot event: " << networkRequest.UUID;
            return Status::CANCELLED;
        }

        //此处为事件触发
        //TODO 此处改成多线程调用

        Netife::PluginsDispatcher::Instance()->ProcessAllPlugins
                ([networkResponse](NetifePlugins* plugins){ plugins->OnResponseTrigger(networkResponse); });

        TransNetWorkResponse(networkResponse, response);

        return Status::OK;
    }

    NetworkRequest NetifeServiceImpl::TransNetifeProbeRequest(const NetifeProbeRequest *request) {
        NetworkRequest req;
        req.UUID = request->uuid();
        switch (request->request_type()) {
            case NetifeMessage::NetifeProbeRequest_RequestType_HTTP:
                req.RequestType = NetworkRequest::HTTP;
                break;
            case NetifeMessage::NetifeProbeRequest_RequestType_HTTPS:
                req.RequestType = NetworkRequest::HTTPS;
                break;
            case NetifeMessage::NetifeProbeRequest_RequestType_WS:
                req.RequestType = NetworkRequest::WS;
                break;
            case NetifeMessage::NetifeProbeRequest_RequestType_WSS:
                req.RequestType = NetworkRequest::WSS;
                break;
            case NetifeMessage::NetifeProbeRequest_RequestType_PING:
                req.RequestType = NetworkRequest::PING;
                break;
            case NetifeMessage::NetifeProbeRequest_RequestType_OTHER:
                req.RequestType = NetworkRequest::OTHER;
                break;
            default:
                break;
        }
        switch (request->application_type()) {

            case NetifeMessage::NetifeProbeRequest_ApplicationType_CLIENT:
                req.ApplicationType = NetworkRequest::CLIENT;
                break;
            case NetifeMessage::NetifeProbeRequest_ApplicationType_SERVER:
                req.ApplicationType = NetworkRequest::SERVER;
                break;
            default:
                break;
        }
        switch (request->protocol()) {
            case NetifeMessage::NetifeProbeRequest_Protocol_TCP:
                req.Protocol = NetworkRequest::TCP;
                break;
            case NetifeMessage::NetifeProbeRequest_Protocol_UDP:
                req.Protocol = NetworkRequest::UDP;
                break;
            default:
                break;
        }
        req.DstIpAddr = request->dst_ip_addr();
        req.DstIpPort = request->dst_ip_port();
        req.SrcIpAddr = request->src_ip_addr();
        req.SrcIpPort = request->src_ip_port();
        req.UUIDSub = request->has_uuid_sub() ? make_optional<int>(request->uuid_sub()) : nullopt;
        req.IsRawText = request->is_raw_text();
        req.Pid = request->has_pid() ? make_optional<string>(request->pid()) : nullopt;
        req.RawText = request->raw_text();
        req.ProcessName = request->has_process_name() ? make_optional<string>(request->process_name()) : nullopt;
        return req;
    }

    NetworkResponse NetifeServiceImpl::TransNetifeProbeResponse(const NetifeProbeResponse *response) {
        NetworkResponse networkResponse;
        networkResponse.UUID = response->uuid();
        networkResponse.DstIpPort = response->dst_ip_port();
        networkResponse.DstIpAddr = response->dst_ip_addr();
        networkResponse.ResponseText = response->response_text();
        return networkResponse;
    }

    void NetifeServiceImpl::TransNetWorkRequest(NetworkRequest networkRequest, NetifeProbeRequest* request) {
        request->set_uuid(networkRequest.UUID);
        switch (networkRequest.RequestType) {
            case NetworkRequest::HTTP:
                request->set_request_type(NetifeMessage::NetifeProbeRequest_RequestType_HTTP);
                break;
            case NetworkRequest::HTTPS:
                request->set_request_type(NetifeMessage::NetifeProbeRequest_RequestType_HTTPS);
                break;
            case NetworkRequest::WS:
                request->set_request_type(NetifeMessage::NetifeProbeRequest_RequestType_WS);
                break;
            case NetworkRequest::WSS:
                request->set_request_type(NetifeMessage::NetifeProbeRequest_RequestType_WSS);
                break;
            case NetworkRequest::PING:
                request->set_request_type(NetifeMessage::NetifeProbeRequest_RequestType_PING);
                break;
            case NetworkRequest::OTHER:
                request->set_request_type(NetifeMessage::NetifeProbeRequest_RequestType_OTHER);
                break;
        }
        switch (networkRequest.ApplicationType) {
            case NetworkRequest::CLIENT:
                request->set_application_type(NetifeMessage::NetifeProbeRequest_ApplicationType_CLIENT);
                break;
            case NetworkRequest::SERVER:
                request->set_application_type(NetifeMessage::NetifeProbeRequest_ApplicationType_SERVER);
                break;
        }
        switch (networkRequest.Protocol) {
            case NetworkRequest::TCP:
                request->set_protocol(NetifeMessage::NetifeProbeRequest_Protocol_TCP);
                break;
            case NetworkRequest::UDP:
                request->set_protocol(NetifeMessage::NetifeProbeRequest_Protocol_UDP);
                break;
        }
        request->set_dst_ip_addr(networkRequest.DstIpAddr);
        request->set_dst_ip_port(networkRequest.DstIpPort);
        request->set_src_ip_addr(networkRequest.SrcIpAddr);
        request->set_src_ip_port(networkRequest.SrcIpPort);
        request->set_is_raw_text(networkRequest.IsRawText);
        request->set_raw_text(networkRequest.RawText);
        if (networkRequest.UUIDSub.has_value()){
            request->set_uuid_sub(networkRequest.UUIDSub.value());
        }
        if (networkRequest.Pid.has_value()){
            request->set_pid(networkRequest.Pid.value());
        }
        if (networkRequest.ProcessName.has_value()){
            request->set_process_name(networkRequest.ProcessName.value());
        }
    }

    void NetifeServiceImpl::TransNetWorkResponse(NetworkResponse networkResponse, NetifeProbeResponse* response) {
        response->set_uuid(networkResponse.UUID);
        response->set_dst_ip_port(networkResponse.DstIpAddr);
        response->set_dst_ip_addr(networkResponse.DstIpPort);
        response->set_response_text(networkResponse.ResponseText);
    }

    Status NetifeServiceImpl::Command(ServerContext *context, const NetifePluginCommandRequest *request,
                                      NetifePluginCommandResponse *response) {

        vector<string> params;
        params.push_back(request->command_prefix());
        for (const auto &item: request->params()){
            params.push_back(item);
        }

        auto res = Netife::PluginsDispatcher::Instance()->
            UseCommandByVector(request->command_prefix(), params);
        if (!res.has_value()){
            return Status::CANCELLED;
        }else{
            response->set_status(true);
            response->set_result(res.value());
        }
        return Status::OK;
    }
} // Netife