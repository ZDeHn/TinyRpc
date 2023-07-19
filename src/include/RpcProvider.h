#pragma once

#include <muduozdh/TcpServer.h>
#include <muduozdh/EventLoop.h>

#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>

#include <string>
#include <functional>
#include <unordered_map>

namespace tinyRpc{

class RpcProvider{

public:

    void notifyService(google::protobuf::Service *service);

    void start();

private:

    struct ServiceInfo{
        google::protobuf::Service *service;
        std::unordered_map<std::string ,const google::protobuf::MethodDescriptor*> methodMap;
    };

    muduozdh::EventLoop loop_;

    std::unordered_map<std::string, ServiceInfo> serviceMap_;

    void onConnection(const muduozdh::TcpConnectionPtr&);
    void onMessage(const muduozdh::TcpConnectionPtr&, muduozdh::Buffer*, muduozdh::Timestamp);
    void sendRpcResponse(const muduozdh::TcpConnectionPtr&, google::protobuf::Message*);

};

}