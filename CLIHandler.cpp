#include "CLIHandler.h"

Params CLIHandler::parse(int argc, char** argv) {
    Params p;
    po::options_description desc("Допустимые опции");
    desc.add_options()
        ("help,h", "Показать справку")
        ("run,r", "Запуск с параметрами по умолчанию")
        ("address,a", po::value<std::string>(&p.address), "Сетевой адрес сервера")
        ("port,p", po::value<int>(&p.port)->default_value(33333), "Порт сервера")
        ("input,i", po::value<std::string>(&p.inputFile), "Имя файла с исходными данными")
        ("output,o", po::value<std::string>(&p.outputFile), "Имя файла для сохранения результатов ")
        ("config,c", po::value<std::string>(&p.configFile)->default_value("~/.config/vclient.conf"), "Файл с LOGIN и PASSWORD");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (argc == 1 || vm.count("help")) {
        p.help = true;
        return p;
    }

    if (vm.count("run")) {
        if (!vm.count("address")) p.address = "127.0.0.1";
        if (!vm.count("input")) p.inputFile = "input.bin";
        if (!vm.count("output")) p.outputFile = "output.txt";
    } else {
        if (!vm.count("address") || !vm.count("input") || !vm.count("output")) {
            p.help = true;
            return p;
        }
    }

    po::notify(vm); 
    return p;
}

void CLIHandler::printHelp() const {
    std::cout << "Использование: ./client [ОПЦИИ]\n";
    std::cout << "Опции:\n"
              << "  -h, --help       Показать эту справку\n"
              << "  -r, --run        Запуск с параметрами по умолчанию\n"
              << "  -a, --address    Сетевой адрес сервера (обязательный)\n"
              << "  -p, --port       Порт сервера (по умолчанию: 33333)\n"
              << "  -i, --input      Имя файла с исходными данными (обязательный)\n"
              << "  -o, --output     Имя файла для сохранения результатов (обязательный)\n"
              << "  -c, --config     Файл конфигурации (по умолчанию: ~/.config/vclient.conf)\n";
}
