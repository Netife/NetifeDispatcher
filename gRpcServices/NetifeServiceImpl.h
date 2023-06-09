//
// Created by Administrator on 2023/4/30.
//

#ifndef NETIFEDISPATCHER_NETIFESERVICEIMPL_H
#define NETIFEDISPATCHER_NETIFESERVICEIMPL_H
#include <string>
#include <grpcpp/grpcpp.h>
#include "../gRpcModel/NetifeMessage.grpc.pb.h"
#include "../include/models/NetworkRequest.h"
#include "../include/models/NetworkResponse.h"
#include "NetifePostClientImpl.h"
#include "NetifeJsRemoteImpl.h"
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using NetifeMessage::NetifeProbeRequest;
using NetifeMessage::NetifeProbeResponse;
using NetifeMessage::NetifeService;
using NetifeMessage::NetifePluginCommandRequest;
using NetifeMessage::NetifePluginCommandResponse;

namespace Netife {

    class NetifeServiceImpl final : public NetifeService::Service{
    private:
        Netife::NetifePostClientImpl client;
        Netife::NetifeJsRemoteImpl jsRemote;
    public:
        NetifeServiceImpl(const string& name, const std::string& port,const string& jsName, const std::string& jsPort)
            : client(
                    grpc::CreateChannel(name + ":" + port, grpc::InsecureChannelCredentials())),
              jsRemote(grpc::CreateChannel(jsName + ":" + jsPort, grpc::InsecureChannelCredentials())){}
        Status ProcessProbe (ServerContext* context, const NetifeProbeRequest* request, NetifeProbeResponse* response) override;
        static NetworkRequest TransNetifeProbeRequest(const NetifeProbeRequest* request);
        static NetworkResponse TransNetifeProbeResponse(const NetifeProbeResponse* response);
        static void TransNetWorkResponse(NetworkResponse networkResponse, NetifeProbeResponse* response);
        static void TransNetWorkRequest(NetworkRequest networkRequest, NetifeProbeRequest* request);
        Status Command (ServerContext* context, const NetifePluginCommandRequest* request, NetifePluginCommandResponse* response) override;
    };

} // Netife

#endif //NETIFEDISPATCHER_NETIFESERVICEIMPL_H
