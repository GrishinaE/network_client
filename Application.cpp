#include "Application.h"
#include "CLIHandler.h"
#include "DataReader.h"
#include "CryptoAuthModule.h"
#include "ClientServer.h"
#include <fstream>
#include <stdexcept>
#include <cstdlib>

std::string Application::expandPath(const std::string& path) const {
    if (path.length() >= 2 && path[0] == '~' && path[1] == '/') {
        const char* home = getenv("HOME");
        if (home) return std::string(home) + path.substr(1);
    }
    return path;
}

void Application::run(int argc, char** argv) {
    CLIHandler cli;
    Params params = cli.parse(argc, argv);

    if (params.help) {
        cli.printHelp();
        return;
    }

    std::string configPath = expandPath(params.configFile);
    auto creds = DataReader::readConfig(configPath);
    std::string login = creds.first;
    std::string password = creds.second;

    auto data = DataReader::readBinaryData(params.inputFile);

    ClientServer server(params.address, params.port);
    server.connectToServer();

    CryptoAuthModule auth;
    std::string salt = auth.generateSalt();
    std::string hash = auth.generateSHA224(salt + password);
    std::string authMsg = login + salt + hash;

    server.sendString(authMsg);
    std::string authResp = server.receiveString();
    
    if (authResp != "OK") {
        throw std::runtime_error("Application Ошибка аутентификации - " + authResp);
    }

    server.sendData(data);
    auto result = server.receiveResult();

    std::ofstream out(params.outputFile);
    if (!out) throw std::runtime_error("Application Не удалось открыть файл " + params.outputFile);

    out << result.size();
    for (float f : result) {
        out << " " << f;
    }
    out << "\n";
}
