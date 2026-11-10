#include "DataReader.h"
#include <fstream>
#include <stdexcept>
#include <cstdint>

BinaryData DataReader::readBinaryData(const std::string& filename) {
    std::ifstream f(filename, std::ios::binary);
    if (!f) throw std::runtime_error("DataReader Не удалось открыть файл " + filename);

    uint32_t N;
    f.read(reinterpret_cast<char*>(&N), sizeof(N));
    if (!f) throw std::runtime_error("DataReader Не удалось прочитать N");

    BinaryData data(N);
    for (uint32_t i = 0; i < N; ++i) {
        uint32_t S;
        f.read(reinterpret_cast<char*>(&S), sizeof(S));
        if (!f) throw std::runtime_error("DataReader Не удалось прочитать S");
        
        data[i].resize(S);
        f.read(reinterpret_cast<char*>(data[i].data()), S * sizeof(float));
        if (!f) throw std::runtime_error("DataReader Не удалось прочитать данные вектора");
    }
    return data;
}

std::pair<std::string, std::string> DataReader::readConfig(const std::string& filename) {
    std::ifstream f(filename);
    if (!f) throw std::runtime_error("DataReader Не удалось открыть файл " + filename);
    
    std::string line, login, password;
    while (std::getline(f, line)) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            login = line.substr(0, pos);
            password = line.substr(pos + 1);
            break;
        }
    }
    if (login.empty() || password.empty()) 
        throw std::runtime_error("DataReader Неверный формат (ожидается LOGIN:PASSWORD)");
    return {login, password};
}
