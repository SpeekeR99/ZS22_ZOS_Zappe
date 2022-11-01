#include <iostream>
#include <sstream>
#include "commands.h"

command_map init() {
    command_map commands;
    commands["help"] = &Commands::help;
    commands["exit"] = &Commands::exit;
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
    auto commands = init();

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

        if (commands.count(tokens[0]))
            commands[tokens[0]](tokens);
        else
            std::cerr << "Unknown command: " << tokens[0] << std::endl;

    }

    std::cout << "Exiting..." << std::endl;

    return 0;
}
