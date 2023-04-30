#include <iostream>
#include <string>
#include <grpcpp/grpcpp.h>
#include "gRpcModel/NetifeMessage.pb.h"
// 如果是想要调试本程序，请将本值设置为 True
#define DEBUG_MODE true

using namespace std;
int main(int argc, char* argv[]) {

    GOOGLE_PROTOBUF_VERIFY_VERSION; //自检版本

    cout << "[Netife Dispatcher] Netife is running... ..." << endl;

    // 参数自检
    if (argc != 3 && !DEBUG_MODE){
        cout << "[Netife Dispatcher] Netife Dispatch is not a individual component, which need "
                "not only NetifeProbe but also NetifeCore." << endl;
        return -1;
    }

    string hostIp;
    int hostPort;

    if (DEBUG_MODE){
        hostIp = "127.0.0.1"; //调试模式预设默认调试地址
        hostPort = 7899; //调试模式预设默认端口
    }else{
        hostIp = argv[1];
        hostPort = stoi(argv[2]);
    }

    cout << "[Netife Dispatcher] The dispatcher service will run on the " << hostIp << ":" << hostPort << endl;

    return 0;
}