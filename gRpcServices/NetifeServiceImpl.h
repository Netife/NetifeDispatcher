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
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using NetifeMessage::NetifeProbeRequest;
using NetifeMessage::NetifeProbeResponse;
using NetifeMessage::NetifeService;

namespace Netife {

    class NetifeServiceImpl final : public NetifeService::Service{
    private:
        Netife::NetifePostClientImpl client;
    public:
        NetifeServiceImpl(const string& name, const std::string& port)
            : client(
                    grpc::CreateChannel(name + ":" + port, grpc::InsecureChannelCredentials())){}
        Status ProcessProbe (ServerContext* context, const NetifeProbeRequest* request, NetifeProbeResponse* response) override;
        static NetworkRequest TransNetifeProbeRequest(const NetifeProbeRequest* request);
        static NetworkResponse TransNetifeProbeResponse(const NetifeProbeResponse* response);
        static void TransNetWorkResponse(NetworkResponse networkResponse, NetifeProbeResponse* response);
        static void TransNetWorkRequest(NetworkRequest networkRequest, NetifeProbeRequest* request);
    };

} // Netife

#endif //NETIFEDISPATCHER_NETIFESERVICEIMPL_H
