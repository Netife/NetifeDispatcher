//
// Created by Administrator on 2023/5/2.
//

#ifndef NETIFEDISPATCHER_NETIFEJSREMOTEIMPL_H
#define NETIFEDISPATCHER_NETIFEJSREMOTEIMPL_H
#include "../gRpcModel/NetifeMessage.grpc.pb.h"
#include <optional>
using namespace std;
using grpc::ChannelInterface;
using grpc::ClientContext;
using grpc::Status;
using NetifeMessage::NetifeProbeRequest;
using NetifeMessage::NetifeProbeResponse;
using NetifeMessage::NetifePost;
namespace Netife {

    class NetifeJsRemoteImpl {
    private:
        std::unique_ptr<NetifePost::Stub> _stub;
    public:
        explicit NetifeJsRemoteImpl(const std::shared_ptr<ChannelInterface>& channel)
                : _stub(NetifePost::NewStub(channel)) {
        }
        optional<NetifeProbeResponse> ProcessScript(NetifeProbeRequest request);
    };

} // Netife

#endif //NETIFEDISPATCHER_NETIFEJSREMOTEIMPL_H
