#include "pseudofat.h"

PseudoFS::PseudoFS(const std::string &filepath) : file_system_filepath{filepath}, meta_data{"zapped99", 0, DEFAULT_CLUSTER_SIZE, 0, sizeof(struct MetaData), 0} {
    file_system.open(filepath, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
    if (!file_system.is_open())
        std::cerr << "Error opening file" << std::endl;
}

PseudoFS::~PseudoFS() {
    file_system.close();
}

void PseudoFS::format(uint32_t disk_size) {
    meta_data.disk_size = disk_size;
    meta_data.cluster_count = disk_size / meta_data.cluster_size;

    if (file_system.is_open()) file_system.close();
    file_system.open(file_system_filepath, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);

    file_system.write(reinterpret_cast<const char *>(&meta_data), sizeof(struct MetaData));
    for (uint32_t i = meta_data.fat_start_address; i < disk_size; i++)
        file_system << FAT_FREE;
}
