#pragma once
#include <string>

class Application {
private:
    std::string expandPath(const std::string& path) const;

public:
    void run(int argc, char** argv);
};
