#include "include/RpcApplication.h"

#include <iostream>
#include <unistd.h>
#include <string>

using namespace tinyRpc;

RpcConfiguration RpcApplication::configuration;

void showArgsHelp(){
    std::cout << "format: command -i <configuration_file>" << std::endl;
}

void RpcApplication::init(int argc, char** argv){

    if(argc < 2){
        showArgsHelp();
        exit(1);
    }

    int c = 0;
    std::string configurationFilePath;
    while((c = getopt(argc, argv, "i:")) != -1){
        switch (c){
        case 'i':
            configurationFilePath = optarg;
            break;
        case '?':
            showArgsHelp();
            exit(1);
        case ':':
            showArgsHelp();
            exit(1);
        default:
            break;
        }
    }

    configuration.loadConfiguration(configurationFilePath.c_str());

}

RpcApplication& RpcApplication::getInstance(){

    static RpcApplication app;
    return app;
}

RpcConfiguration& RpcApplication::getConfiguration(){
    return configuration;
}
