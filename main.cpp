#include <iostream>
#include <array>
#include <vector>
#include <memory>
#include <sstream>

constexpr size_t NUM_KNOWN_CMDS = 15;
constexpr std::array<const char *, NUM_KNOWN_CMDS> KNOWN_CMDS = {
        "cp",
        "mv",
        "rm",
        "mkdir",
        "rmdir",
        "ls",
        "cat",
        "cd",
        "pwd",
        "info",
        "incp",
        "outcp",
        "load",
        "format",
        "defrag"
};

int cp(const std::vector<std::string> &args) {
    printf("Copy\n");
    return 0;
}

int mv(const std::vector<std::string> &args) {
    printf("Move\n");
    return 0;
}

int rm(const std::vector<std::string> &args) {
    printf("Remove\n");
    return 0;
}

int mkdir(const std::vector<std::string> &args) {
    printf("Make directory\n");
    return 0;
}

int rmdir(const std::vector<std::string> &args) {
    printf("Remove directory\n");
    return 0;
}

int ls(const std::vector<std::string> &args) {
    printf("List files\n");
    return 0;
}

int cat(const std::vector<std::string> &args) {
    printf("Cat std output\n");
    return 0;
}

int cd(const std::vector<std::string> &args) {
    printf("Change directory\n");
    return 0;
}

int pwd(const std::vector<std::string> &args) {
    printf("Print working directory\n");
    return 0;
}

int info(const std::vector<std::string> &args) {
    printf("Info\n");
    return 0;
}

int incp(const std::vector<std::string> &args) {
    printf("Copy in\n");
    return 0;
}

int outcp(const std::vector<std::string> &args) {
    printf("Copy out\n");
    return 0;
}

int load(const std::vector<std::string> &args) {
    printf("Load\n");
    return 0;
}

int format(const std::vector<std::string> &args) {
    printf("Format\n");
    return 0;
}

int defrag(const std::vector<std::string> &args) {
    printf("Defragmentation\n");
    return 0;
}

int main(int argc, char **argv) {
    int (*cmd_call[]) (const std::vector<std::string> &args) = {
            cp,
            mv,
            rm,
            mkdir,
            rmdir,
            ls,
            cat,
            cd,
            pwd,
            info,
            incp,
            outcp,
            load,
            format,
            defrag
    };

    for(;;) {
        std::string input;
        std::getline(std::cin, input);

        if (input == "exit")
            break;

        std::stringstream ss(input);
        std::vector<std::string> tokens;
        std::string token;
        while (std::getline(ss, token, ' '))
            tokens.push_back(token);

        bool unknown_cmd = true;
        for (int i = 0; i < KNOWN_CMDS.size(); i++) {
            if (tokens[0] == std::string(KNOWN_CMDS[i])) {
                cmd_call[i](tokens);
                unknown_cmd = false;
                break;
            }
        }
        if (unknown_cmd)
            std::cerr << "Unknown command: " << tokens[0] << std::endl;
    }

    std::cout << "Exiting..." << std::endl;

    return 0;
}
