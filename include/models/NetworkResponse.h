//
// Created by Administrator on 2023/5/2.
//

#ifndef NETIFEDISPATCHER_NETWORKRESPONSE_H
#define NETIFEDISPATCHER_NETWORKRESPONSE_H
#include <string>
using namespace std;
class NetworkResponse{
    string uuid; // UUID 回显，本 UUID 与gRpc请求的 UUID相同

    string dst_ip_addr; // 返回修改后的发送 IP 地址

    string dst_ip_port; // 返回修改后的发送 PORT 地址

    string response_text; // 返回修改后的 ResponseText。
                              // HTTP / HTTPS 对应的是 Raw Request，WS / WSS 对应的是 String 或 Binary Stream 的 Hex String 表示
};
#endif //NETIFEDISPATCHER_NETWORKRESPONSE_H
