#include "commands.h"

int Commands::help(const std::unique_ptr<PseudoFS>& fs, const std::vector<std::string> &args) {
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
    return 0;
}

int Commands::cp(const std::unique_ptr<PseudoFS>& fs, const std::vector<std::string> &args) {
    std::cout << "Copy" << std::endl;
    return 0;
}

int Commands::mv(const std::unique_ptr<PseudoFS>& fs, const std::vector<std::string> &args) {
    std::cout << "Move" << std::endl;
    return 0;
}

int Commands::rm(const std::unique_ptr<PseudoFS>& fs, const std::vector<std::string> &args) {
    std::cout << "Remove" << std::endl;
    return 0;
}

int Commands::mkdir(const std::unique_ptr<PseudoFS>& fs, const std::vector<std::string> &args) {
    std::cout << "Make directory" << std::endl;
    return 0;
}

int Commands::rmdir(const std::unique_ptr<PseudoFS>& fs, const std::vector<std::string> &args) {
    std::cout << "Remove directory" << std::endl;
    return 0;
}

int Commands::ls(const std::unique_ptr<PseudoFS>& fs, const std::vector<std::string> &args) {
    std::cout << "List" << std::endl;
    return 0;
}

int Commands::cat(const std::unique_ptr<PseudoFS>& fs, const std::vector<std::string> &args) {
    std::cout << "Cat" << std::endl;
    return 0;
}

int Commands::cd(const std::unique_ptr<PseudoFS>& fs, const std::vector<std::string> &args) {
    std::cout << "Change directory" << std::endl;
    return 0;
}

int Commands::pwd(const std::unique_ptr<PseudoFS>& fs, const std::vector<std::string> &args) {
    std::cout << "Print working directory" << std::endl;
    return 0;
}

int Commands::info(const std::unique_ptr<PseudoFS>& fs, const std::vector<std::string> &args) {
    std::cout << "Info" << std::endl;
    return 0;
}

int Commands::incp(const std::unique_ptr<PseudoFS>& fs, const std::vector<std::string> &args) {
    std::cout << "In copy" << std::endl;
    return 0;
}

int Commands::outcp(const std::unique_ptr<PseudoFS>& fs, const std::vector<std::string> &args) {
    std::cout << "Out copy" << std::endl;
    return 0;
}

int Commands::load(const std::unique_ptr<PseudoFS>& fs, const std::vector<std::string> &args) {
    std::cout << "Load" << std::endl;
    return 0;
}

int Commands::format(const std::unique_ptr<PseudoFS>& fs, const std::vector<std::string> &args) {
    fs->format(std::stoi(args[1]));
    std::cout << "Formatted successfully to " << args[1] << " bytes" << std::endl;
    return 0;
}

int Commands::defrag(const std::unique_ptr<PseudoFS>& fs, const std::vector<std::string> &args) {
    std::cout << "Defrag" << std::endl;
    return 0;
}
