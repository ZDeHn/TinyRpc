#include "include/RpcChannel.h"
#include "include/ZookeeperUtil.h"
#include "include/RpcHeader.pb.h"
#include "include/RpcApplication.h"
#include "include/RpcChannel.h"

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>


using namespace tinyRpc;

void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method, google::protobuf::RpcController* controller, const google::protobuf::Message* request, google::protobuf::Message* response, google::protobuf:: Closure* done){
                                    
    const google::protobuf::ServiceDescriptor* sd = method->service();

    std::string serviceName = sd->name(); 
    std::string methodName = method->name();

    uint32_t argsSize = 0;
    std::string argsStr;
    if (request->SerializeToString(&argsStr)){
        argsSize = argsStr.size();
    }
    else{
        controller->SetFailed("serialize request error!");
        return;
    }
    
    RpcHeader rpcHeader;
    rpcHeader.set_service_name(serviceName);
    rpcHeader.set_method_name(methodName);
    rpcHeader.set_args_size(argsSize);

    uint32_t headerSize = 0;
    std::string rpcHeadrStr;

    if (rpcHeader.SerializeToString(&rpcHeadrStr)){
        headerSize = rpcHeadrStr.size();
    }
    else{
        controller->SetFailed("Serialize Rpc Header Error!");
        return;
    }

    std::string sendRpcStr;
    sendRpcStr.insert(0, std::string((char*)&headerSize, 4));
    sendRpcStr += rpcHeadrStr; 
    sendRpcStr += argsStr; 

    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd){

        char errtxt[512] = {0};
        sprintf(errtxt, "create socket error! errno:%d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    ZookeeperUtil zookeeperUtil;
    zookeeperUtil.start();

    std::string methodPath = "/" + serviceName + "/" + methodName;

    std::string hostData = zookeeperUtil.get(methodPath.c_str());

    if (hostData == ""){
        controller->SetFailed(methodPath + " is not exist!");
        return;
    }
    int idx = hostData.find(":");
    if (idx == -1){
        controller->SetFailed(methodPath + " address is invalid!");
        return;
    }
    std::string ip = hostData.substr(0, idx);
    uint16_t port = atoi(hostData.substr(idx + 1, hostData.size()-idx).c_str()); 

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (-1 == connect(clientfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr))){

        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "connect error! errno:%d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    if (-1 == send(clientfd, sendRpcStr.c_str(), sendRpcStr.size(), 0)){

        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "send error! errno:%d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    char recvBuf[1024] = {0};
    int recvSize = 0;

    if (-1 == (recvSize = recv(clientfd, recvBuf, 1024, 0))){

        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "recv error! errno:%d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    if (!response->ParseFromArray(recvBuf, recvSize)){
        
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "parse error! response_str:%s", recvBuf);
        controller->SetFailed(errtxt);
        return;
    }

    close(clientfd);
}