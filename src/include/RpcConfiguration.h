#pragma once

#include <unordered_map>
#include <string>

namespace tinyRpc{

class RpcConfiguration{

public:

    void loadConfiguration(const char* filePath);
    std::string get(const std::string &key);
    
private:

    std::unordered_map<std::string, std::string> configurationMap_;
    void trim(std::string &srcBuf);

};

}