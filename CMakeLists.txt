cmake_minimum_required(VERSION 3.25)
project(NetifeDispatcher)

set(CMAKE_CXX_STANDARD 17)

# 支持编译 gRpc
find_package(gRPC CONFIG REQUIRED)

find_package(Protobuf REQUIRED)

add_executable(NetifeDispatcher main.cpp
        gRpcModel/NetifeMessage.pb.cc gRpcModel/NetifeMessage.grpc.pb.h gRpcModel/NetifeMessage.grpc.pb.cc gRpcModel/NetifeMessage.pb.h)

target_link_libraries(NetifeDispatcher PRIVATE
        gRPC::grpc++ gRPC::grpc++_reflection gRPC::gpr gRPC::grpc
        protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)