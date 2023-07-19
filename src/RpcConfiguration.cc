#include "include/RpcConfiguration.h"

#include <muduozdh/Logging.h>

#include <iostream>
#include <string>

using namespace tinyRpc;

void RpcConfiguration::loadConfiguration(const char* filePath){

    FILE *pf = fopen(filePath, "r");
    
    if(pf == nullptr){
        std::cout << filePath << " is not exists !" << std::endl;
        exit(1);
    }

    while(!feof(pf)){

        char buf[512] = {0};
        fgets(buf, 512, pf);

        std::string readBuf(buf);
        trim(readBuf);

        if (readBuf[0] == '#' || readBuf.empty()){
            continue;
        }

        int equalIndex = readBuf.find('=');
        if (equalIndex == -1){
            continue;
        }

        int endIndex = readBuf.find('\n');
        if(endIndex == -1){
            endIndex = readBuf.size();
        }

        std::string key;
        std::string value;

        key = readBuf.substr(0, equalIndex);
        trim(key);
        value = readBuf.substr(equalIndex + 1, endIndex - equalIndex - 1);
        trim(value);

        configurationMap_.insert({key, value});

        LOG << "Configuration insert key: " << key << " value: " << value; 

    }

    fclose(pf);

}


std::string RpcConfiguration::get(const std::string &key){

    auto it = configurationMap_.find(key);
    if(it == configurationMap_.end()){
        return std::string();
    }
    else{
        std::string value = it->second;
        return value;
    }
}

void RpcConfiguration::trim(std::string &srcBuf){

    int spaceIndex;
    
    spaceIndex = srcBuf.find_first_not_of(' ');
    if(spaceIndex != -1){
        srcBuf = srcBuf.substr(spaceIndex, srcBuf.size() - spaceIndex);
    }

    spaceIndex = srcBuf.find_last_not_of(' ');
    if(spaceIndex != -1){
        srcBuf = srcBuf.substr(0,spaceIndex + 1);
    }
    
}
