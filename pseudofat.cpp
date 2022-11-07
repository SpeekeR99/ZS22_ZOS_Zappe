#include "pseudofat.h"

PseudoFS::PseudoFS(const std::string &filepath) : file_system_filepath{filepath}, meta_data{"zapped99", 0, DEFAULT_CLUSTER_SIZE, 0, sizeof(struct MetaData), 0} {
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

void PseudoFS::help(const std::vector<std::string> &tokens) {
    std::cout << "-------------------------------------------------------------------------------" << std::endl;
    std::cout << "| help              | display this message                                    |" << std::endl;
    std::cout << "| exit              | exit the program                                        |" << std::endl;
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
    uint32_t disk_size = std::stoi(args[1]);
    if (args[1].find("KB") != std::string::npos)
        disk_size *= KB;
    else if (args[1].find("MB") != std::string::npos)
        disk_size *= MB;
    else if (args[1].find("GB") != std::string::npos)
        disk_size *= GB;

    meta_data.disk_size = disk_size;
    meta_data.cluster_count = disk_size / meta_data.cluster_size;

    if (file_system.is_open()) file_system.close();
    file_system.open(file_system_filepath, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);

    file_system.write(reinterpret_cast<const char *>(&meta_data), sizeof(struct MetaData));
    for (uint32_t i = meta_data.fat_start_address; i < disk_size; i++)
        file_system << FAT_FREE;

    std::cout << "OK (Formatted successfully to " << disk_size << " bytes)" << std::endl;
}

void PseudoFS::defrag(const std::vector<std::string> &args) {

}
