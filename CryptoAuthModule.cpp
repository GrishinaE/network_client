#include "CryptoAuthModule.h"
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/osrng.h>
#include <cryptopp/filters.h>
#include <cstdio>
#include <cstdint>

std::string CryptoAuthModule::generateSalt() {
    CryptoPP::AutoSeededRandomPool prng;
    uint64_t salt;
    prng.GenerateBlock(reinterpret_cast<CryptoPP::byte*>(&salt), sizeof(salt));
    
    char buf[17];
    std::snprintf(buf, sizeof(buf), "%016lx", salt); 
    return std::string(buf);
}

std::string CryptoAuthModule::generateSHA224(const std::string& msg) {
    CryptoPP::SHA224 sha;
    std::string hash;
    CryptoPP::StringSource(msg, true,
        new CryptoPP::HashFilter(sha,
            new CryptoPP::HexEncoder(
                new CryptoPP::StringSink(hash), true)));
    return hash;
}
