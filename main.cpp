#include <memory>
#include <iostream>
#include "pseudofat.h"

int main(int argc, char **argv) {
    std::unique_ptr<PseudoFS> fs = std::make_unique<PseudoFS>("myfs.dat");

    std::string token;
    std::vector<std::string> tokens;
    for (;;) {
        std::cout << fs->get_working_directory() << "$ >" << std::flush;
        std::string input;
        std::getline(std::cin, input);

        if (input == "exit")
            break;

        std::stringstream ss(input);
        tokens.clear();
        while (std::getline(ss, token, ' '))
            tokens.push_back(token);

        fs->call_cmd(tokens[0], tokens);
    }

    std::cout << "Exiting..." << std::endl;

    return 0;
}
