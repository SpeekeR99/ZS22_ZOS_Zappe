#pragma once

#include <iostream>
#include <fstream>

/** Constant representing unused cluster */
constexpr int32_t FAT_FREE = -1;
/** Constant representing end of file */
constexpr int32_t FAT_EOF = -2;
/** Constant representing bad cluster */
constexpr int32_t FAT_BAD = -3;
/** 1024 Bytes = 1 KB */
constexpr int32_t KB = 1024;
/** 1024 KB = 1 MB */
constexpr int32_t MB = 1024 * KB;
/** 1024 MB = 1 GB */
constexpr int32_t GB = 1024 * MB;

/**
 * Class representing a pseudo FAT file system
 * The file system is stored in a file on the disk
 */
class PseudoFS {
private:
    std::string file_system_filepath;
    std::fstream file_system;

public:
    explicit PseudoFS(const std::string &filepath);
    ~PseudoFS();

    void format(uint32_t disk_size);
};
