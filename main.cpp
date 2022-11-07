#include <memory>
#include <iostream>
#include <sstream>
#include "pseudofat.h"

int main(int argc, char **argv) {
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

        fs->call_cmd(tokens[0], tokens);
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
