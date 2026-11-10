#pragma once
#include <string>
#include <utility>
#include "Types.h"

class DataReader {
public:
    static BinaryData readBinaryData(const std::string& filename);
    static std::pair<std::string, std::string> readConfig(const std::string& filename);
};
