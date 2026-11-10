#pragma once
#include <string>

class CryptoAuthModule {
public:
    std::string generateSalt();
    std::string generateSHA224(const std::string& msg);
};
