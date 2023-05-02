//
// Created by Administrator on 2023/4/30.
//

#ifndef NETIFEDISPATCHER_NETIFEPOSTCLIENTIMPL_H
#define NETIFEDISPATCHER_NETIFEPOSTCLIENTIMPL_H
#include "../gRpcModel/NetifeMessage.grpc.pb.h"

using grpc::ChannelInterface;
using grpc::ClientContext;
using grpc::Status;
using NetifeMessage::NetifeProbeRequest;
using NetifeMessage::NetifeProbeResponse;
using NetifeMessage::NetifePost;

namespace Netife {

    class NetifePostClientImpl {
    private:
        std::unique_ptr<NetifePost::Stub> _stub;
    public:
        explicit NetifePostClientImpl(const std::shared_ptr<ChannelInterface>& channel)
            : _stub(NetifePost::NewStub(channel)) {
        }


    };

} // Netife

#endif //NETIFEDISPATCHER_NETIFEPOSTCLIENTIMPL_H
