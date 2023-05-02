#include <iostream>
#include <string>
#include <grpcpp/grpcpp.h>
#include "gRpcServices/NetifeServiceImpl.h"
#include "utils/logger.h"
#include "lib/log/easylogging++.h"
#include "plugins/v1/PluginsDispatcher.h"
#include <filesystem>

INITIALIZE_EASYLOGGINGPP;

// 如果是想要调试本程序，请将本值设置为 True
#define DEBUG_MODE true
#define DEBUG_HOST_IP "0.0.0.0"
#define DEBUG_HOST_PORT "7890"

using namespace std;
namespace fs = std::filesystem;


void StartServer(const string &host, const string &port);
void createDirIfNotExists(string dirName);
void InitDirectoryStructure();

int main(int argc, char *argv[]) {
    GOOGLE_PROTOBUF_VERIFY_VERSION; //自检版本

    //日志加载

    Netife::logger::RegisterCommonLogger("Loader");
    Netife::logger::RegisterCommonLogger("PluginsDispatcher");
    Netife::logger::RegisterCommonLogger("Plugin");
    Netife::logger::RegisterCommonLogger("NetifeService");
    //初始化目录结构

    InitDirectoryStructure();

    //加载插件
    CLOG(INFO, "Loader") << "Netife is loading plugins......";
    Netife::PluginsDispatcher::Instance()->AutoLoadPlugins();

    CLOG(INFO, "Loader") << "Netife is running......";
    // 参数自检f
    if (argc != 3 && !DEBUG_MODE) {
        CLOG(INFO, "Loader") << "Netife Dispatch is not a individual component, which need "
                "not only NetifeProbe but also NetifeCore.";
        return -1;
    }

    string hostIp = DEBUG_HOST_IP;
    string hostPort = DEBUG_HOST_PORT;

    if (!DEBUG_MODE) {
        hostIp = argv[1];
        hostPort = stoi(argv[2]);
    }

    // 启动服务器

    StartServer("0.0.0.0", "7890");
    return 0;
}

void StartServer(const string &host, const string &port) {
    CLOG(INFO, "Loader") << "The dispatcher service will run on the " << host << ":" << port;
    Netife::NetifeServiceImpl service;
    ServerBuilder builder;
    builder.AddListeningPort(host + ":" + port, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    server->Wait();
}

void InitDirectoryStructure() {
    createDirIfNotExists("plugins");
    createDirIfNotExists("logs");
    createDirIfNotExists("plugins\\bin");
    createDirIfNotExists("plugins\\config");
    createDirIfNotExists("plugins\\data");
    createDirIfNotExists("config");
}

void createDirIfNotExists(string dirName){
    if (!fs::exists(dirName) && !fs::is_directory(dirName)) {
        fs::create_directory(dirName);
    }
}