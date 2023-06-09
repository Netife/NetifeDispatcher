cmake_minimum_required(VERSION 3.25)

project(NetifeDispatcher VERSION 1.0.0
                         DESCRIPTION "NetifeDispatcher, see at https://github.com/Netife/NetifeDispatcher"
                         LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

# gRpc
find_package(gRPC REQUIRED)
find_package(Protobuf REQUIRED)
# Poco
find_package(Poco REQUIRED Foundation)
find_package(Poco REQUIRED JSON)
#sqlite3
find_package(unofficial-sqlite3 CONFIG REQUIRED)

if (WIN32)
    add_definitions(/utf-8)
endif()
add_executable(NetifeDispatcher main.cpp
        gRpcModel/NetifeMessage.pb.cc gRpcModel/NetifeMessage.pb.h
        gRpcModel/NetifeMessage.grpc.pb.cc gRpcModel/NetifeMessage.grpc.pb.h
        gRpcServices/NetifeServiceImpl.cpp gRpcServices/NetifeServiceImpl.h
        gRpcServices/NetifePostClientImpl.cpp gRpcServices/NetifePostClientImpl.h
        lib/log/easylogging++.cc lib/log/easylogging++.h
        utils/logger.cpp utils/logger.h include/plugins/v1/NetifePlugins.h
        plugins/v1/PluginsDispatcher.cpp plugins/v1/PluginsDispatcher.h
        plugins/v1/PluginsDescriptor.cpp plugins/v1/PluginsDescriptor.h

        utils/TextHelper.cpp utils/TextHelper.h plugins/v1/CommandDescriptor.cpp
        plugins/v1/CommandDescriptor.h plugins/v1/HookDescriptor.cpp plugins/v1/HookDescriptor.h include/models/NetworkRequest.h include/models/NetworkResponse.h plugins/v1/NetifeAgentImpl.cpp plugins/v1/NetifeAgentImpl.h plugins/v1/ScriptDescriptor.cpp plugins/v1/ScriptDescriptor.h gRpcServices/NetifeJsRemoteImpl.cpp gRpcServices/NetifeJsRemoteImpl.h services/NetifeStorage.cpp services/NetifeStorage.h)

# gRpc
target_link_libraries(NetifeDispatcher PRIVATE
        gRPC::grpc++ gRPC::grpc++_reflection gRPC::gpr gRPC::grpc
        protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)

# Poco
target_link_libraries(NetifeDispatcher PRIVATE Poco::Foundation)
target_link_libraries(NetifeDispatcher PRIVATE Poco::JSON)

# sqlite3
target_link_libraries(NetifeDispatcher PRIVATE unofficial::sqlite3::sqlite3)