#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

/** Free cluster constant */
constexpr int8_t FAT_FREE = -1;
/** End of file constant */
constexpr int8_t FAT_EOF = -2;
/** Bad cluster constant */
constexpr int8_t FAT_BAD = -3;
/** 1024 Bytes = 1 KB */
constexpr int32_t KB = 1024;
/** 1024 KB = 1 MB */
constexpr int32_t MB = 1024 * KB;
/** 1024 MB = 1 GB */
constexpr int32_t GB = 1024 * MB;
/** Cluster size in bytes */
constexpr uint32_t DEFAULT_CLUSTER_SIZE = 1 * KB;

/**
 * MetaData structure for the whole file system
 * Includes information about the file system
 */
struct MetaData {
    /** zapped99 + null terminator = 9 */
    char signature[9];
    /** Size of the file system in bytes */
    uint32_t disk_size;
    /** Size of a cluster in bytes */
    uint32_t cluster_size;
    /** Number of clusters in the file system */
    uint32_t cluster_count;
    /** Fat table offset in bytes */
    uint32_t fat_start_address;
    /** Root directory offset in bytes */
    uint32_t data_start_address;
};

/**
 * Class representing a pseudo FAT file system
 * The file system is stored in a file on the disk
 */
class PseudoFS {
private:
    typedef void (PseudoFS::*command)(const std::vector<std::string> &);
    typedef std::map<std::string, command> command_map;
    command_map commands;
    std::string file_system_filepath;
    std::fstream file_system;
    struct MetaData meta_data;

    void initialize_command_map();
    void help(const std::vector<std::string> &tokens);
    void cp(const std::vector<std::string> &tokens);
    void mv(const std::vector<std::string> &args);
    void rm(const std::vector<std::string> &args);
    void mkdir(const std::vector<std::string> &args);
    void rmdir(const std::vector<std::string> &args);
    void ls(const std::vector<std::string> &args);
    void cat(const std::vector<std::string> &args);
    void cd(const std::vector<std::string> &args);
    void pwd(const std::vector<std::string> &args);
    void info(const std::vector<std::string> &args);
    void incp(const std::vector<std::string> &args);
    void outcp(const std::vector<std::string> &args);
    void load(const std::vector<std::string> &args);
    void format(const std::vector<std::string> &args);
    void defrag(const std::vector<std::string> &args);

public:
    explicit PseudoFS(const std::string &filepath);
    ~PseudoFS();

    void call_cmd(const std::string &cmd, const std::vector<std::string> &args);
};
