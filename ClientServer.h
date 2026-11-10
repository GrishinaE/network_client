#pragma once
#include <string>
#include <vector>
#include "Types.h"

class ClientServer {
private:
    int sockfd = -1;
    std::string serverAddress;
    int serverPort;

public:
    ClientServer(const std::string& addr, int port);
    ~ClientServer();

    void connectToServer();
    void sendString(const std::string& msg);
    std::string receiveString();
    void sendData(const BinaryData& data);
    std::vector<float> receiveResult();
};
