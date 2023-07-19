#pragma once

#include "RpcConfiguration.h"

namespace tinyRpc{

class RpcApplication{

public:

    static void init(int argc, char** argv);
    static RpcApplication& getInstance();
    static RpcConfiguration& getConfiguration();

private:

    RpcApplication() {};
    RpcApplication(const RpcApplication&) = delete;
    RpcApplication(RpcApplication&&) = delete;

    static RpcConfiguration configuration;

};

}