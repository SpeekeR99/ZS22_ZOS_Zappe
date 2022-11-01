#include "commands.h"

int Commands::help(const std::vector<std::string> &args) {
    std::cout << "Help is on the way" << std::endl;
    return 0;
}

int Commands::exit(const std::vector<std::string> &args) {
    return 0;
}

int Commands::cp(const std::vector<std::string> &args) {
    std::cout << "Copy" << std::endl;
    return 0;
}

int Commands::mv(const std::vector<std::string> &args) {
    std::cout << "Move" << std::endl;
    return 0;
}

int Commands::rm(const std::vector<std::string> &args) {
    std::cout << "Remove" << std::endl;
    return 0;
}

int Commands::mkdir(const std::vector<std::string> &args) {
    std::cout << "Make directory" << std::endl;
    return 0;
}

int Commands::rmdir(const std::vector<std::string> &args) {
    std::cout << "Remove directory" << std::endl;
    return 0;
}

int Commands::ls(const std::vector<std::string> &args) {
    std::cout << "List" << std::endl;
    return 0;
}

int Commands::cat(const std::vector<std::string> &args) {
    std::cout << "Cat" << std::endl;
    return 0;
}

int Commands::cd(const std::vector<std::string> &args) {
    std::cout << "Change directory" << std::endl;
    return 0;
}

int Commands::pwd(const std::vector<std::string> &args) {
    std::cout << "Print working directory" << std::endl;
    return 0;
}

int Commands::info(const std::vector<std::string> &args) {
    std::cout << "Info" << std::endl;
    return 0;
}

int Commands::incp(const std::vector<std::string> &args) {
    std::cout << "In copy" << std::endl;
    return 0;
}

int Commands::outcp(const std::vector<std::string> &args) {
    std::cout << "Out copy" << std::endl;
    return 0;
}

int Commands::load(const std::vector<std::string> &args) {
    std::cout << "Load" << std::endl;
    return 0;
}

int Commands::format(const std::vector<std::string> &args) {
    std::cout << "Format" << std::endl;
    return 0;
}

int Commands::defrag(const std::vector<std::string> &args) {
    std::cout << "Defrag" << std::endl;
    return 0;
}
