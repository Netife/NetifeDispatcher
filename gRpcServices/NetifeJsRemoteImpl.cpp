//
// Created by Administrator on 2023/5/2.
//

#include "NetifeJsRemoteImpl.h"

namespace Netife {
    optional<NetifeProbeResponse> NetifeJsRemoteImpl::ProcessScript(NetifeProbeRequest request) {
        //插件处理
        NetifeProbeResponse response;
        ClientContext context;
        Status status = _stub->UploadRequest(&context, request, &response);
        if (status.ok()) {
            return response;
        } else {
            return nullopt;
        }
    }
} // Netife