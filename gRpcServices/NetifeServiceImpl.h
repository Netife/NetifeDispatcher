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

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using NetifeMessage::NetifeProbeRequest;
using NetifeMessage::NetifeProbeResponse;
using NetifeMessage::NetifeService;

namespace Netife {

    class NetifeServiceImpl final : public NetifeService::Service{
    public:
        Status ProcessProbe (ServerContext* context, const NetifeProbeRequest* request, NetifeProbeResponse* response) override;
        static NetworkRequest TransNetifeProbeRequest(const NetifeProbeRequest* request);
        static NetworkResponse TransNetifeProbeResponse(const NetifeProbeResponse* response);
        static void TransNetifeProbeResponse(NetworkResponse networkResponse, NetifeProbeResponse* response);
        static void TransNetifeProbeRequest(NetworkRequest networkRequest, NetifeProbeRequest* request);
    };

} // Netife

#endif //NETIFEDISPATCHER_NETIFESERVICEIMPL_H
