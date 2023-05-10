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
#define DEBUG_FRONTEND_IP "localhost"
#define DEBUG_FRONTEND_PORT "7891"
#define DEBUG_JS_REMOTE_IP "localhost"
#define DEBUG_JS_REMOTE_PORT "7892"

using namespace std;
namespace fs = std::filesystem;


void StartServer(const string &host, const string &port, const string& clientHost, const string& clientPort,
            const string &jsIp, const string &jsPort);
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
    Netife::PluginsDispatcher::Instance()->AutoLoadScripts();
    CLOG(INFO, "Loader") << "Netife is running......";
    // 参数自检f
    if (argc != 7 && !DEBUG_MODE) {
        CLOG(INFO, "Loader") << "Netife Dispatch is not a individual component, which need "
                "not only NetifeProbe but also NetifeCore.";
        return -1;
    }

    string hostIp = DEBUG_HOST_IP;
    string hostPort = DEBUG_HOST_PORT;
    string clientIp = DEBUG_FRONTEND_IP;
    string clientPort = DEBUG_FRONTEND_PORT;
    string jsIp = DEBUG_JS_REMOTE_IP;
    string jsPort = DEBUG_JS_REMOTE_PORT;

    if (!DEBUG_MODE) {
        hostIp = argv[1];
        hostPort = argv[2];
        clientIp = argv[3];
        clientPort = argv[4];
        jsIp = argv[5];
        jsPort = argv[6];
    }

    // 启动服务器

    StartServer(hostIp, hostPort, clientIp, clientPort, jsIp, jsPort);
    return 0;
}

void StartServer(const string &host, const string &port, const string& clientHost, const string& clientPort,
                 const string &jsIp, const string &jsPort) {
    CLOG(INFO, "Loader") << "The dispatcher service will run on the " << host << ":" << port;
    Netife::NetifeServiceImpl service(clientHost, clientPort, jsIp, jsPort);
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
    createDirIfNotExists("scripts");
    createDirIfNotExists("scripts\\bin");
    createDirIfNotExists("scripts\\data");
    createDirIfNotExists("scripts\\config");
}

void createDirIfNotExists(string dirName){
    if (!fs::exists(dirName) && !fs::is_directory(dirName)) {
        fs::create_directory(dirName);
    }
}