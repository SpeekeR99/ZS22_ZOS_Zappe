#include "pseudofat.h"

PseudoFS::PseudoFS(const std::string &filepath) : file_system_filepath{filepath}, meta_data{} {
    file_system.open(filepath, std::ios::binary | std::ios::in | std::ios::out);
    if (!file_system.is_open())
        file_system.open(filepath, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
    if (!file_system.is_open())
        std::cerr << "Error opening file system file" << std::endl;
    initialize_command_map();
}

PseudoFS::~PseudoFS() {
    file_system.close();
}

void PseudoFS::initialize_command_map() {
    commands["help"] = &PseudoFS::help;
    commands["meta"] = &PseudoFS::meta;
    commands["cp"] = &PseudoFS::cp;
    commands["mv"] = &PseudoFS::mv;
    commands["rm"] = &PseudoFS::rm;
    commands["mkdir"] = &PseudoFS::mkdir;
    commands["rmdir"] = &PseudoFS::rmdir;
    commands["ls"] = &PseudoFS::ls;
    commands["cat"] = &PseudoFS::cat;
    commands["cd"] = &PseudoFS::cd;
    commands["pwd"] = &PseudoFS::pwd;
    commands["info"] = &PseudoFS::info;
    commands["incp"] = &PseudoFS::incp;
    commands["outcp"] = &PseudoFS::outcp;
    commands["load"] = &PseudoFS::load;
    commands["format"] = &PseudoFS::format;
    commands["defrag"] = &PseudoFS::defrag;
}

void PseudoFS::call_cmd(const std::string &cmd, const std::vector<std::string> &args) {
    if (commands.count(cmd))
        (this->*commands[cmd])(args);
    else {
        std::cerr << "Unknown command: " << cmd << std::endl;
        std::cerr << "Type 'help' for a list of commands" << std::endl;
    }
}

void PseudoFS::help(const std::vector<std::string> &args) {
    std::cout << "-------------------------------------------------------------------------------" << std::endl;
    std::cout << "| help              | display this message                                    |" << std::endl;
    std::cout << "| exit              | exit the program                                        |" << std::endl;
    std::cout << "| meta              | display meta information about the file system          |" << std::endl;
    std::cout << "| cp <src> <dst>    | copy file from <src> to <dst>                           |" << std::endl;
    std::cout << "| mv <src> <dst>    | move file from <src> to <dst>                           |" << std::endl;
    std::cout << "| rm <file>         | remove file <file>                                      |" << std::endl;
    std::cout << "| mkdir <dir>       | create directory <dir>                                  |" << std::endl;
    std::cout << "| rmdir <dir>       | remove directory <dir>                                  |" << std::endl;
    std::cout << "| ls <dir>          | list directory <dir> contents                           |" << std::endl;
    std::cout << "| cat <file>        | display file <file> contents                            |" << std::endl;
    std::cout << "| cd <dir>          | change current directory to <dir>                       |" << std::endl;
    std::cout << "| pwd               | print working directory                                 |" << std::endl;
    std::cout << "| info <dir/file>   | display information about directory <dir> / file <file> |" << std::endl;
    std::cout << "| incp <src> <dst>  | copy file from disk <src> to <dst> in the file system   |" << std::endl;
    std::cout << "| outcp <src> <dst> | copy file from <src> in the file system to disk <dst>   |" << std::endl;
    std::cout << "| load <file>       | load file <file> from disk and execute commands from it |" << std::endl;
    std::cout << "| format <size>     | format the file system with size <size>                 |" << std::endl;
    std::cout << "| defrag <file>     | defragment the file <file>                              |" << std::endl;
    std::cout << "-------------------------------------------------------------------------------" << std::endl;
}

void PseudoFS::meta(const std::vector<std::string> &args) {
    struct MetaData meta{};
    std::ifstream input_stream = std::ifstream(file_system_filepath, std::ios::binary);
    input_stream.read(reinterpret_cast<char *>(&meta), sizeof(MetaData));
    std::cout << "-------------------------------------------------------------------------------" << std::endl;
    std::cout << "Signature:          " << meta.signature << std::endl;
    std::cout << "Disk size:          " << meta.disk_size << std::endl;
    std::cout << "Cluster size:       " << meta.cluster_size << std::endl;
    std::cout << "Cluster count:      " << meta.cluster_count << std::endl;
    std::cout << "Fat start address:  " << meta.fat_start_address << std::endl;
    std::cout << "Fat size:           " << meta.fat_size << std::endl;
    std::cout << "Data start address: " << meta.data_start_address << std::endl;
    std::cout << "-------------------------------------------------------------------------------" << std::endl;
    input_stream.close();
}

void PseudoFS::cp(const std::vector<std::string> &tokens) {

}

void PseudoFS::mv(const std::vector<std::string> &args) {

}

void PseudoFS::rm(const std::vector<std::string> &args) {

}

void PseudoFS::mkdir(const std::vector<std::string> &args) {

}

void PseudoFS::rmdir(const std::vector<std::string> &args) {

}

void PseudoFS::ls(const std::vector<std::string> &args) {

}

void PseudoFS::cat(const std::vector<std::string> &args) {

}

void PseudoFS::cd(const std::vector<std::string> &args) {

}

void PseudoFS::pwd(const std::vector<std::string> &args) {

}

void PseudoFS::info(const std::vector<std::string> &args) {

}

void PseudoFS::incp(const std::vector<std::string> &args) {

}

void PseudoFS::outcp(const std::vector<std::string> &args) {

}

void PseudoFS::load(const std::vector<std::string> &args) {

}

void PseudoFS::format(const std::vector<std::string> &args) {
    // Get the user input for the disk size
    uint32_t disk_size = std::stoi(args[1]);
    if (args[1].find("KB") != std::string::npos)
        disk_size *= KB;
    else if (args[1].find("MB") != std::string::npos)
        disk_size *= MB;
    else if (args[1].find("GB") != std::string::npos)
        disk_size *= GB;

    // Calculate remaining size (size for FAT table and data)
    uint32_t remaining_size = disk_size - sizeof(MetaData);
    uint32_t num_blocks = remaining_size / (DEFAULT_CLUSTER_SIZE + sizeof(uint32_t));

    // Create the meta data for the file system
    meta_data = MetaData{
            "zapped99",
            disk_size,
            DEFAULT_CLUSTER_SIZE,
            (disk_size - (sizeof(MetaData) + num_blocks * sizeof(uint32_t))) / DEFAULT_CLUSTER_SIZE,
            sizeof(MetaData),
            num_blocks * sizeof(uint32_t),
            sizeof(MetaData) + num_blocks * sizeof(uint32_t)
    };
    // Create root directory
    auto root_dir = DirectoryEntry{
        ".",
        true,
        0,
        meta_data.data_start_address,
    };
    auto root_dir_parent = DirectoryEntry{
        "..",
        true,
        0,
        meta_data.data_start_address,
    };

    // Rewrite the file system file
    if (file_system.is_open()) file_system.close();
    file_system.open(file_system_filepath, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);

    // Write the meta data
    file_system.write(reinterpret_cast<const char *>(&meta_data), sizeof(struct MetaData));

    // Write the FAT table (all clusters are free)
    for (uint32_t i = 0; i < meta_data.cluster_count; i++)
        file_system.write(reinterpret_cast<const char *>(&FAT_FREE), sizeof(uint32_t));

    // Write the data (no data)
    const char *nothing = "\0";
    for (uint32_t i = 0; i < meta_data.cluster_count * meta_data.cluster_size; i++)
        file_system.write(nothing, sizeof(char));

    // Write the root directory to FAT table and data
    file_system.seekp(meta_data.fat_start_address);
    file_system.write(reinterpret_cast<const char *>(&meta_data.data_start_address), sizeof(uint32_t));
    file_system.seekp(meta_data.data_start_address);
    file_system.write(reinterpret_cast<const char *>(&root_dir), sizeof(DirectoryEntry));
    file_system.write(reinterpret_cast<const char *>(&root_dir_parent), sizeof(DirectoryEntry));

    std::cout << "OK" << std::endl;
}

void PseudoFS::defrag(const std::vector<std::string> &args) {

}
