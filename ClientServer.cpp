#include "ClientServer.h"
#include <stdexcept>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdint>

ClientServer::ClientServer(const std::string& addr, int port) 
    : serverAddress(addr), serverPort(port) {}

ClientServer::~ClientServer() {
    if (sockfd >= 0) close(sockfd);
}

void ClientServer::connectToServer() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) throw std::runtime_error("ClientServer socket() failed");

    struct timeval timeout{180, 0};
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serverPort);
    
    if (inet_pton(AF_INET, serverAddress.c_str(), &serv_addr.sin_addr) <= 0) {
        throw std::runtime_error("ClientServer Неверный адрес " + serverAddress);
    }
    
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        throw std::runtime_error("ClientServer connect() failed");
    }
}

void ClientServer::sendString(const std::string& msg) {
    if (send(sockfd, msg.c_str(), msg.length(), 0) < 0) 
        throw std::runtime_error("ClientServer send() failed");
}

std::string ClientServer::receiveString() {
    char buf[1024];
    int rc = recv(sockfd, buf, sizeof(buf) - 1, 0);
    if (rc < 0) throw std::runtime_error("ClientServer recv() failed");
    buf[rc] = '\0';
    return std::string(buf);
}

void ClientServer::sendData(const BinaryData& data) {
    uint32_t N = data.size();
    if (send(sockfd, &N, sizeof(N), 0) < 0) 
        throw std::runtime_error("ClientServer Не удалось отправить N");

    for (const auto& vec : data) {
        uint32_t S = vec.size();
        if (send(sockfd, &S, sizeof(S), 0) < 0) 
            throw std::runtime_error("ClientServer Не удалось отправить S");
        if (send(sockfd, vec.data(), S * sizeof(float), 0) < 0) 
            throw std::runtime_error("ClientServer Не удалось отправить вектор");
    }
}
std::vector<float> ClientServer::receiveResult() {
    std::vector<float> res;
    float val;
    while (true) {
        ssize_t rc = recv(sockfd, &val, sizeof(val), MSG_WAITALL);
        if (rc == 0) {
            break; 
        }
        if (rc != sizeof(val)) {
            throw std::runtime_error("ClientServer: Не удалось получить результаты");
        }
        res.push_back(val);
    }
    return res;
}
