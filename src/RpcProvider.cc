#include "include/RpcProvider.h"
#include "include/RpcApplication.h"
#include "include/ZookeeperUtil.h"
#include "include/RpcHeader.pb.h"

#include <muduozdh/Logging.h>
#include <muduozdh/InetAddress.h>

#include <functional>

using namespace tinyRpc;

void RpcProvider::notifyService(google::protobuf::Service *service){

    ServiceInfo info;
    const google::protobuf::ServiceDescriptor* serviceDescriptor = service->GetDescriptor();

    std::string serviceName = serviceDescriptor->name();
    int methodCount = serviceDescriptor->method_count();

    LOG << "Service Name: " << serviceName;

    for(int i = 0; i < methodCount ; i++){

        const google::protobuf::MethodDescriptor* methodDescriptor = serviceDescriptor->method(i);
        std::string methodName = methodDescriptor->name();
        info.methodMap.insert({methodName, methodDescriptor});

        LOG << "Method Name:" << methodName;
    }

    info.service = service;
    LOG << "insert";
    serviceMap_.insert({serviceName, info});

}

void RpcProvider::start(){

    std::string ip = RpcApplication::getInstance().getConfiguration().get("rpc_server_ip");
    uint16_t port = atoi(RpcApplication::getInstance().getConfiguration().get("rpc_server_port").c_str());

    LOG << "Rpc Servier ip:" << ip << " Port:" << port;

    std::cout << ip << std::endl;

    muduozdh::InetAddress address(port, ip);
    muduozdh::TcpServer server(&loop_, address, "RpcProvider", muduozdh::TcpServer::kReusePort);

    server.setConnectionCallback(std::bind(&RpcProvider::onConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    server.setThreadNum(4);

    ZookeeperUtil zookeeperUtil;
    zookeeperUtil.start();

    for (auto &sp : serviceMap_){

        std::string servicePath = "/" + sp.first;
        zookeeperUtil.create(servicePath.c_str(), nullptr, 0);
        
        LOG << "Rpc Provider Creating Service Node:";

        for(auto &mp : sp.second.methodMap){

            std::string methodPath =servicePath + "/" +  mp.first;
            char methodPathData[128] = {0};
            sprintf(methodPathData, "%s:%d", ip.c_str(), port);
            zookeeperUtil.create(methodPath.c_str(), methodPathData, strlen(methodPathData), ZOO_EPHEMERAL);
        }
    }

    LOG << "RpcServer Start at ip: " << ip << " port: " << port;

    server.start();
    loop_.loop();

}

void RpcProvider::onConnection(const muduozdh::TcpConnectionPtr &conn){

    if(!conn->connected()){
        conn->shutdown();
    }
}

void RpcProvider::onMessage(const muduozdh::TcpConnectionPtr &conn, muduozdh::Buffer* buf,muduozdh::Timestamp receiveTime){

    std::string request(buf->peek(), buf->readableBytes());

    uint32_t headerSize = 0;
    if(request.size() < 4){

        LOG << "Received Buffer is Incomplete";
        return ;
    }

    request.copy((char *)&headerSize, 4, 0);

    if(request.size() < 4 + headerSize){
        
        LOG << "Received Buffer is Incomplete";
        return ;
    }

    std::string rpcHeaderString = request.substr(4, headerSize);
    
    RpcHeader rpcHeader;
    std::string serviceName;
    std::string methodName;
    uint32_t argsSize;
    
    if(rpcHeader.ParseFromString(rpcHeaderString)){
        serviceName = rpcHeader.service_name();
        methodName = rpcHeader.method_name();
        argsSize = rpcHeader.args_size();
    }

    else{

        LOG << "Parse Rpc Header Error!";
        buf->retrieveUntil(buf->peek() + 4 + headerSize);
        return ;
    }

    if(request.size() < 4 + headerSize + argsSize){

        LOG << "Received Buffer is Incomplete";
        return ;
    }

    std::string argsStr = request.substr(4 + headerSize, argsSize);

    buf->retrieveUntil(buf->peek() + 4 + headerSize + argsSize);

    auto sit = serviceMap_.find(serviceName);

    if(sit == serviceMap_.end()){
        LOG << "Service: "<<serviceName << " is not Exists!";
        conn->forceClose();
        // conn->shutdown();
        return ;
    }

    auto mit = sit->second.methodMap.find(methodName);
    if(mit == sit->second.methodMap.end()){
        LOG << "Method: " << methodName << " is not Exists!";
        conn->forceClose();
        // conn->shutdown();
        return ;
    }

    google::protobuf::Service* service = sit->second.service;
    const google::protobuf::MethodDescriptor* method = mit->second;

    google::protobuf::Message* requestMessage = service->GetRequestPrototype(method).New();
    if(!requestMessage->ParseFromString(argsStr)){
        LOG << "Parse Request Args Error, Content: " << argsStr ;
        conn->forceClose();
        // conn->shutdown();
        return ;
    }

    google::protobuf::Message* responseMessage = service->GetResponsePrototype(method).New();
    
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider, const muduozdh::TcpConnectionPtr&, google::protobuf::Message*>(this, &RpcProvider::sendRpcResponse, conn, responseMessage);

    service->CallMethod(method, nullptr, requestMessage, responseMessage, done);

} 

void RpcProvider::sendRpcResponse(const muduozdh::TcpConnectionPtr& conn, google::protobuf::Message *responseMessage){

    std::string responseStr;
    if(responseMessage->SerializeToString(&responseStr)){

        conn->send(responseStr);
    }
    else{
        LOG << "Serialize Response Error!";
        conn->forceClose();
        // conn->shutdown();
    }
    conn->shutdown();

}








