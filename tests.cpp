#include <UnitTest++/UnitTest++.h>
#include <fstream>
#include <cstdio>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "CLIHandler.h"
#include "DataReader.h"
#include "CryptoAuthModule.h"
#include "ClientServer.h"
#include "Types.h"

#define CHECK_EQUAL_STR(WS,RS) { \
    std::string waiting = WS; \
    std::string resulting = RS; \
    REQUIRE CHECK_EQUAL(waiting.size(),resulting.size()); \
    CHECK_EQUAL(waiting,resulting); \
}

// 1. ТЕСТЫ ДЛЯ CLIHandler
SUITE(CLIHandlerTest) {
    TEST(HelpFlag) {
        CLIHandler cli;
        const char* argv[] = {"client", "-h", nullptr};
        int argc = sizeof(argv) / sizeof(nullptr) - 1;
        Params p = cli.parse(argc, const_cast<char**>(argv));
        CHECK(p.help == true);
    }

    TEST(RunDefaultFlag) {
        CLIHandler cli;
        const char* argv[] = {"client", "-r", nullptr};
        int argc = sizeof(argv) / sizeof(nullptr) - 1;
        Params p = cli.parse(argc, const_cast<char**>(argv));
        CHECK(p.help == false);
        CHECK_EQUAL_STR("127.0.0.1", p.address);
        CHECK_EQUAL_STR("input.bin", p.inputFile);
        CHECK_EQUAL_STR("output.txt", p.outputFile);
    }

    TEST(MissingArgsTriggersHelp) {
        CLIHandler cli;
        const char* argv[] = {"client", "-a", "192.168.1.1", nullptr}; 
        int argc = sizeof(argv) / sizeof(nullptr) - 1;
        Params p = cli.parse(argc, const_cast<char**>(argv));
        CHECK(p.help == true);
    }
}

// 2. ТЕСТЫ ДЛЯ DataReader
struct BinaryDataFixture {
    const std::string validFile = "test_valid_data.bin";
    const std::string configFile = "test_config.conf";

    BinaryDataFixture() {
        std::ofstream f(validFile, std::ios::binary);
        uint32_t N = 1, S = 3;
        float data[] = {1.0f, 2.0f, 3.0f};
        f.write(reinterpret_cast<char*>(&N), sizeof(N));
        f.write(reinterpret_cast<char*>(&S), sizeof(S));
        f.write(reinterpret_cast<char*>(data), sizeof(data));
        f.close();

        std::ofstream f2(configFile);
        f2 << "user:P@ssW0rd\n";
    }
    ~BinaryDataFixture() {
        std::remove(validFile.c_str());
        std::remove(configFile.c_str());
    }
};

SUITE(DataReaderTest) {
    TEST(NoFile) {
        CHECK_THROW(DataReader::readBinaryData("not_exist.bin"), std::runtime_error);
    }
    TEST_FIXTURE(BinaryDataFixture, ValidData) {
        BinaryData data = DataReader::readBinaryData(validFile);
        REQUIRE CHECK_EQUAL(1, data.size());
        REQUIRE CHECK_EQUAL(3, data[0].size());
        CHECK_CLOSE(1.0f, data[0][0], 0.001f);
    }
    TEST_FIXTURE(BinaryDataFixture, ValidConfig) {
        auto creds = DataReader::readConfig(configFile);
        CHECK_EQUAL_STR("user", creds.first);
        CHECK_EQUAL_STR("P@ssW0rd", creds.second);
    }
}

// 3. ТЕСТЫ ДЛЯ CryptoAuthModule
SUITE(CryptoAuthModuleTest) {
    TEST(SaltLength) {
        CryptoAuthModule auth;
        std::string salt = auth.generateSalt();
        CHECK_EQUAL(16, salt.size());
    }
    TEST(HashGeneration) {
        CryptoAuthModule auth;
        std::string expected = "90A3ED9E32B2AAF4C61C410EB925426119E1A9DC53D4286ADE99A809";
        CHECK_EQUAL_STR(expected, auth.generateSHA224("test"));
    }
}

// 4. ТЕСТЫ ДЛЯ ClientServer
struct FakeServer {
    int port = 45678;
    int server_fd;
    std::thread server_thread;

    FakeServer() {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        bind(server_fd, (struct sockaddr*)&address, sizeof(address));
        listen(server_fd, 1);
        
        server_thread = std::thread([this]() {
            int client_socket = accept(server_fd, nullptr, nullptr);
            if (client_socket >= 0) {
                char buf[1024] = {0};
                recv(client_socket, buf, sizeof(buf), 0);
                send(client_socket, "OK", 2, 0);
                
                uint32_t N; recv(client_socket, &N, sizeof(N), MSG_WAITALL);
                uint32_t S; recv(client_socket, &S, sizeof(S), MSG_WAITALL);
                std::vector<float> data(S);
                recv(client_socket, data.data(), S * sizeof(float), MSG_WAITALL);
                
                float sum = 0;
                for(float v : data) sum += v;
                send(client_socket, &sum, sizeof(sum), 0);
                
                close(client_socket);
            }
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    ~FakeServer() {
        if(server_thread.joinable()) server_thread.join();
        close(server_fd);
    }
};

SUITE(ClientServerTest) {
    TEST(ConnectionRefused) {
        ClientServer comm("127.0.0.1", 9999);
        CHECK_THROW(comm.connectToServer(), std::runtime_error);
    }

    TEST_FIXTURE(FakeServer, FullExchange) {
        ClientServer comm("127.0.0.1", port);
        
        comm.connectToServer();
        
        comm.sendString("HELLO");
        CHECK_EQUAL_STR("OK", comm.receiveString());
        
        BinaryData data = {{1.0f, 2.0f, 3.0f}};
        comm.sendData(data);
        
        std::vector<float> res = comm.receiveResult();
        REQUIRE CHECK_EQUAL(1, res.size());
        CHECK_CLOSE(6.0f, res[0], 0.001f);
    }
}

int main() {
    return UnitTest::RunAllTests();
}
