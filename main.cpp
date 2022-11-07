#include <memory>
#include <iostream>
#include <sstream>
#include "commands.h"
#include "pseudofat.h"

command_map init_cmd_map() {
    command_map commands;
    commands["help"] = &Commands::help;
    commands["cp"] = &Commands::cp;
    commands["mv"] = &Commands::mv;
    commands["rm"] = &Commands::rm;
    commands["mkdir"] = &Commands::mkdir;
    commands["rmdir"] = &Commands::rmdir;
    commands["ls"] = &Commands::ls;
    commands["cat"] = &Commands::cat;
    commands["cd"] = &Commands::cd;
    commands["pwd"] = &Commands::pwd;
    commands["info"] = &Commands::info;
    commands["incp"] = &Commands::incp;
    commands["outcp"] = &Commands::outcp;
    commands["load"] = &Commands::load;
    commands["format"] = &Commands::format;
    commands["defrag"] = &Commands::defrag;
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

    return 0;
}
