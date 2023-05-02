//
// Created by Administrator on 2023/4/30.
//

#ifndef NETIFEDISPATCHER_NETIFESERVICEIMPL_H
#define NETIFEDISPATCHER_NETIFESERVICEIMPL_H
#include <string>
#include <grpcpp/grpcpp.h>
#include "../gRpcModel/NetifeMessage.grpc.pb.h"

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
    };

} // Netife

#endif //NETIFEDISPATCHER_NETIFESERVICEIMPL_H
