#include "Application.h"
#include <iostream>
#include <exception>

int main(int argc, char** argv) {
    try {
        Application app;
        app.run(argc, argv);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Критическая ошибка: " << e.what() << std::endl;
        return 1;
    }
}

