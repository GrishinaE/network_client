#pragma once
#include <string>
#include <iostream>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

struct Params {
    std::string address;
    int port = 33333;
    std::string inputFile;
    std::string outputFile;
    std::string configFile = "~/.config/vclient.conf";
    bool help = false;
};

class CLIHandler {
public:
    Params parse(int argc, char** argv);
    void printHelp() const;
};
