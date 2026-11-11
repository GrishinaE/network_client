#include <UnitTest++/UnitTest++.h>
#include <iostream>
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
    TEST(Test1) {
        CLIHandler cli;
        const char* argv[] = {"client", "-h", nullptr};
        Params p = cli.parse(2, const_cast<char**>(argv));
        std::cout << "1: справка=" << (p.help ? "true" : "false") << std::endl;
        CHECK(p.help == true);
    }
    TEST(Test2) {
        CLIHandler cli;
        const char* argv[] = {"client", "--help", nullptr};
        Params p = cli.parse(2, const_cast<char**>(argv));
        std::cout << "2: справка=" << (p.help ? "true" : "false") << std::endl;
        CHECK(p.help == true);
    }
    TEST(Test3) {
        CLIHandler cli;
        const char* argv[] = {"client", "-r", nullptr};
        Params p = cli.parse(2, const_cast<char**>(argv));
        std::cout << "3: адрес=" << p.address << ", входной файл=" << p.inputFile << std::endl;
        CHECK(p.help == false);
        CHECK_EQUAL_STR("127.0.0.1", p.address);
        CHECK_EQUAL_STR("input.bin", p.inputFile);
    }
    TEST(Test4) {
        CLIHandler cli;
        const char* argv[] = {"client", "-a", "192.168.1.1", nullptr}; 
        Params p = cli.parse(3, const_cast<char**>(argv));
        std::cout << "4: справка=" << (p.help ? "true" : "false") << std::endl;
        CHECK(p.help == true);
    }
    TEST(Test5) {
        CLIHandler cli;
        const char* argv[] = {"client", "-x", "123", nullptr};
        CHECK_THROW(cli.parse(3, const_cast<char**>(argv)), po::error);
        std::cout << "5: исключение (ошибка параметров)" << std::endl;
    }
}

// 2. ТЕСТЫ ДЛЯ DataReader
struct ReaderFixture {
    const std::string validFile = "test_valid.bin";
    const std::string emptyFile = "test_empty.bin";
    const std::string corruptFile = "test_corrupt.bin";
    const std::string validConf = "test_valid.conf";
    const std::string invalidConf = "test_invalid.conf";

    ReaderFixture() {
        std::ofstream f1(validFile, std::ios::binary);
        uint32_t N = 1, S = 3;
        float data[] = {1.0f, 2.0f, 3.0f};
        f1.write(reinterpret_cast<char*>(&N), sizeof(N));
        f1.write(reinterpret_cast<char*>(&S), sizeof(S));
        f1.write(reinterpret_cast<char*>(data), sizeof(data));
        f1.close();

        std::ofstream f2(emptyFile, std::ios::binary);
        uint32_t N0 = 0;
        f2.write(reinterpret_cast<char*>(&N0), sizeof(N0));
        f2.close();

        std::ofstream f3(corruptFile, std::ios::binary);
        uint32_t N1 = 1, S5 = 5;
        f3.write(reinterpret_cast<char*>(&N1), sizeof(N1));
        f3.write(reinterpret_cast<char*>(&S5), sizeof(S5));
        f3.close();

        std::ofstream f4(validConf); f4 << "user:P@ssW0rd\n"; f4.close();
        std::ofstream f5(invalidConf); f5 << "user P@ssW0rd\n"; f5.close(); 
    }
    ~ReaderFixture() {
        std::remove(validFile.c_str()); std::remove(emptyFile.c_str());
        std::remove(corruptFile.c_str()); std::remove(validConf.c_str());
        std::remove(invalidConf.c_str());
    }
};

SUITE(DataReaderTest) {
    TEST(Test6) {
        CHECK_THROW(DataReader::readBinaryData("not_exist.bin"), std::runtime_error);
        std::cout << "6: исключение (ошибка выполнения)" << std::endl;
    }
    TEST_FIXTURE(ReaderFixture, Test7) {
        BinaryData data = DataReader::readBinaryData(validFile);
        std::cout << "7: векторов=" << data.size() << ", элементов=" << data[0].size() << std::endl;
        REQUIRE CHECK_EQUAL(1, data.size());
        REQUIRE CHECK_EQUAL(3, data[0].size());
    }
    TEST_FIXTURE(ReaderFixture, Test8) {
        BinaryData data = DataReader::readBinaryData(emptyFile);
        std::cout << "8: векторов=" << data.size() << std::endl;
        CHECK_EQUAL(0, data.size());
    }
    TEST_FIXTURE(ReaderFixture, Test9) {
        CHECK_THROW(DataReader::readBinaryData(corruptFile), std::runtime_error);
        std::cout << "9: исключение (ошибка выполнения)" << std::endl;
    }
    TEST_FIXTURE(ReaderFixture, Test10) {
        auto creds = DataReader::readConfig(validConf);
        std::cout << "10: логин=" << creds.first << ", пароль=" << creds.second << std::endl;
        CHECK_EQUAL_STR("user", creds.first);
        CHECK_EQUAL_STR("P@ssW0rd", creds.second);
    }
    TEST_FIXTURE(ReaderFixture, Test11) {
        CHECK_THROW(DataReader::readConfig(invalidConf), std::runtime_error);
        std::cout << "11: исключение (ошибка выполнения)" << std::endl;
    }
}

// 3. ТЕСТЫ ДЛЯ CryptoAuthModule
SUITE(CryptoAuthModuleTest) {
    TEST(Test12) {
        CryptoAuthModule auth;
        std::string s = auth.generateSalt();
        std::cout << "12: длина соли=" << s.size() << " (" << s << ")" << std::endl;
        CHECK_EQUAL(16, s.size());
    }
    TEST(Test13) {
        CryptoAuthModule auth;
        std::string s1 = auth.generateSalt();
        std::string s2 = auth.generateSalt();
        std::cout << "13: уникальность=" << (s1 != s2 ? "true" : "false") << std::endl;
        CHECK(s1 != s2);
    }
    TEST(Test14) {
        CryptoAuthModule auth;
        std::string actual = auth.generateSHA224("test");
        std::cout << "14: хеш=" << actual << std::endl;
        CHECK_EQUAL_STR("90A3ED9E32B2AAF4C61C410EB925426119E1A9DC53D4286ADE99A809", actual);
    }
    TEST(Test15) {
        CryptoAuthModule auth;
        std::string expected = "D14A028C2A3A2BC9476102BB288234C415A2B01F828EA62AC5B3E42F"; 
        std::string actual = auth.generateSHA224("");
        std::cout << "15: хеш пустой строки=" << actual << std::endl;
        CHECK_EQUAL_STR(expected, actual);
    }
}

// 4. ТЕСТЫ ДЛЯ ClientServer
class FakeServer {
public:
    int port;
    int server_fd;
    std::thread server_thread;

    FakeServer(int p) : port(p) {
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
                if (N > 0) {
                    uint32_t S; recv(client_socket, &S, sizeof(S), MSG_WAITALL);
                    std::vector<float> data(S);
                    recv(client_socket, data.data(), S * sizeof(float), MSG_WAITALL);
                    float sum = 0;
                    for(float v : data) sum += v;
                    send(client_socket, &sum, sizeof(sum), 0);
                }
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
    TEST(Test16) {
        ClientServer comm("127.0.0.1", 9999);
        CHECK_THROW(comm.connectToServer(), std::runtime_error);
        std::cout << "16: исключение (ошибка выполнения)" << std::endl;
    }
    TEST(Test17) {
        FakeServer server(45678);
        ClientServer comm("127.0.0.1", 45678);
        comm.connectToServer();
        comm.sendString("HELLO");
        std::string ans = comm.receiveString();
        
        BinaryData data = {{1.0f, 2.0f, 3.0f}};
        comm.sendData(data);
        auto res = comm.receiveResult();
        std::cout << "17: ответ=" << ans << ", сумма=" << res[0] << std::endl;
        
        CHECK_EQUAL_STR("OK", ans);
        REQUIRE CHECK_EQUAL(1, res.size());
        CHECK_CLOSE(6.0f, res[0], 0.001f);
    }
    TEST(Test18) {
        FakeServer server(45679);
        ClientServer comm("127.0.0.1", 45679);
        comm.connectToServer();
        comm.sendString("HELLO");
        comm.receiveString();
        
        BinaryData emptyData = {};
        comm.sendData(emptyData); 
        std::cout << "18: отправка пустых данных=успешно" << std::endl;
        CHECK(true);
    }
    TEST(Test19) {
        FakeServer server(45680);
        ClientServer comm("127.0.0.1", 45680);
        comm.connectToServer();
        comm.sendString("TEST_MSG");
        std::string ans = comm.receiveString();
        std::cout << "19: ответ=" << ans << std::endl;
        CHECK_EQUAL_STR("OK", ans);
    }
}

int main() {
    return UnitTest::RunAllTests();
}
