#include <memory>
#include <iostream>
#include <sstream>
#include "commands.h"
#include "pseudofat.h"

command_map init_cmd_map() {
    command_map commands;
    commands["help"] = &help;
    commands["cp"] = &cp;
    commands["mv"] = &mv;
    commands["rm"] = &rm;
    commands["mkdir"] = &mkdir;
    commands["rmdir"] = &rmdir;
    commands["ls"] = &ls;
    commands["cat"] = &cat;
    commands["cd"] = &cd;
    commands["pwd"] = &pwd;
    commands["info"] = &info;
    commands["incp"] = &incp;
    commands["outcp"] = &outcp;
    commands["load"] = &load;
    commands["format"] = &format;
    commands["defrag"] = &defrag;
    return commands;
}

int main(int argc, char **argv) {
    auto commands = init_cmd_map();
    std::unique_ptr<PseudoFS> fs = std::make_unique<PseudoFS>("myfs.dat");

    std::string token;
    std::vector<std::string> tokens;
    for(;;) {
        std::string input;
        std::getline(std::cin, input);

        if (input == "exit")
            break;

        std::stringstream ss(input);
        tokens.clear();
        while (std::getline(ss, token, ' '))
            tokens.push_back(token);

        if (commands.count(tokens[0]))
            commands[tokens[0]](fs, tokens);
        else {
            std::cerr << "Unknown command: " << tokens[0] << std::endl;
            std::cerr << "Type 'help' for a list of commands" << std::endl;
        }
    }

    std::cout << "Exiting..." << std::endl;

    struct MetaData meta{};
    std::ifstream infs = std::ifstream("myfs.dat", std::ios::binary);
    infs.read(reinterpret_cast<char *>(&meta), sizeof(MetaData));
    std::cout << meta.signature << std::endl;
    std::cout << meta.disk_size << std::endl;
    std::cout << meta.cluster_size << std::endl;
    std::cout << meta.cluster_count << std::endl;
    std::cout << meta.fat_start_address << std::endl;
    std::cout << meta.data_start_address << std::endl;

    return 0;
}
