#include "include/ZookeeperUtil.h"
#include "include/RpcApplication.h"

#include <muduozdh/Logging.h>

using namespace tinyRpc;

void globalWatcher(zhandle_t *zh, int type, int state, const char* path, void* watcherCtx){

    if(type == ZOO_SESSION_EVENT){

        if(state == ZOO_CONNECTED_STATE){
            sem_t *sem = (sem_t*)zoo_get_context(zh);
            sem_post(sem);
        }

    }

}

ZookeeperUtil::ZookeeperUtil() : zhandle_(nullptr) { }
ZookeeperUtil::~ZookeeperUtil(){

    if(zhandle_){
        zookeeper_close(zhandle_);
    }
}

void ZookeeperUtil::start(){

    std::string host = RpcApplication::getInstance().getConfiguration().get("zookeeper_ip");
    std::string port = RpcApplication::getInstance().getConfiguration().get("zookeeper_port");

    std::string hostPort = host + ":" + port;

    zhandle_ = zookeeper_init(hostPort.c_str(), globalWatcher, 10000, nullptr, nullptr, 0);

    if(nullptr == zhandle_){
        LOG << "zookeeper init fail!";
        exit(1);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(zhandle_, &sem);

    sem_wait(&sem);

    LOG << "zookeeper init success";

}

void ZookeeperUtil::create(const char* path, const char* data, int length,int state){

    char pathBuffer[128];
    int bufferLength = sizeof(pathBuffer);

    if(zoo_exists(zhandle_, path, 0, nullptr) == ZNONODE){

        if(zoo_create(zhandle_, path, data, length, &ZOO_OPEN_ACL_UNSAFE, state, pathBuffer, bufferLength) == ZOK){
            LOG << "zookeeper create node: " << path << " success!";
        }
        else{
            LOG << "zookeeper create node: " << path << " fail!";
            exit(1);
        }      
    }
    else{
        LOG << "zookeeper node: " << path << " exists!";
    }
}

std::string ZookeeperUtil::get(const char* path){

    char buffer[64];
    int bufferLength = sizeof(buffer);

    if(zoo_get(zhandle_, path, 0, buffer, &bufferLength, nullptr) == ZOK){
        LOG << "zookeeper get node: " << path << " value: " << buffer;
        return buffer;
    }
    else{
        LOG << "zookeeper get node: " << path << " fail";
        return "";
    }

}



