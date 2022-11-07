#include "pseudofat.h"

PseudoFS::PseudoFS(const std::string &filepath) : file_system_filepath{filepath} {
    file_system.open(filepath, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
    if (!file_system.is_open())
        std::cerr << "Error opening file" << std::endl;
}

PseudoFS::~PseudoFS() {
    file_system.close();
}

void PseudoFS::format(uint32_t disk_size) {
    if (file_system.is_open()) file_system.close();
    file_system.open(file_system_filepath, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
    for (int i = 0; i < disk_size; i++)
        file_system << '\0';
}
