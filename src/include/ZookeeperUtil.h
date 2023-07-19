#pragma once 

// #include <zookeeper/zookeeper.h>

#include <zookeeper/zookeeper.h>

#include <semaphore.h>
#include <string>

namespace tinyRpc{

class ZookeeperUtil{

public:

    ZookeeperUtil();
    ~ZookeeperUtil();

    void start();
    void create(const char* path, const char* data, int length, int state = 0);
    std::string get(const char* path);

private:

    zhandle_t* zhandle_;

};

}