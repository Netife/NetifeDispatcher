//
// Created by Administrator on 2023/5/2.
//

#ifndef NETIFEDISPATCHER_NETWORKREQUEST_H
#define NETIFEDISPATCHER_NETWORKREQUEST_H

#include <string>
#include <optional>

using namespace std;
class NetworkRequest{
public:

    string UUID ; // 全局唯一 UUID，如果是 HTTP / HTTPS 要求响应和回复需要对应唯一；如果是 WS / WSS 那么要求一个通信道中 UUID 均相同

    enum RequestType {
        HTTP = 0, // HTTP协议
        HTTPS = 1, // HTTPS协议
        WS = 2, // WS协议
        WSS = 3, // WSS协议
        PING = 4, // PING
        OTHER = 5, //[本字段保留以便于扩展] 其他
    };

    RequestType RequestType;

    //数据标注
    enum ApplicationType {
        CLIENT = 0, // 表示本消息为客户端请求
        SERVER = 1, // 表示本消息为服务器响应
    };

    ApplicationType ApplicationType;

    //协议类型
    enum Protocol {
        TCP = 0,
        UDP = 1,
    };

    Protocol Protocol;

    string DstIpAddr; // 数据包的原始云端 IP 地址

    string DstIpPort; // 数据包的原始云端 PORT 地址

    string SrcIpAddr; // 数据包的来源 IP 地址

    string SrcIpPort; // 数据包的来源 PORT

    bool IsRawText; // 如果内容不为 Binary Stream 本布尔为 True

    optional<int> UUIDSub; // 如果是 WS / WSS ，那么此表示的是发送顺序，且标号是单独的

    // 如果是 HTTP / HTTPS 那么就是 Raw Text，如果是 WS / WSS 那么就是 字符数据 或 Binary Steam 的 Hex String 表示

    string RawText;

    optional<string> Pid; // [本字段可选] 如果可以抓到进程信息，那么本为其对应的 PID

    optional<string> ProcessName; // [本字段可选] 如果可以抓到进程信息，那么本为其对应的进程名称
};
#endif //NETIFEDISPATCHER_NETWORKREQUEST_H
