#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <string>

typedef int (*command)(const std::vector<std::string> &);
typedef std::map<std::string, command> command_map;

class Commands {
public:
    static int help(const std::vector<std::string> &args);
    static int exit(const std::vector<std::string> &args);
    static int cp(const std::vector<std::string> &args);
    static int mv(const std::vector<std::string> &args);
    static int rm(const std::vector<std::string> &args);
    static int mkdir(const std::vector<std::string> &args);
    static int rmdir(const std::vector<std::string> &args);
    static int ls(const std::vector<std::string> &args);
    static int cat(const std::vector<std::string> &args);
    static int cd(const std::vector<std::string> &args);
    static int pwd(const std::vector<std::string> &args);
    static int info(const std::vector<std::string> &args);
    static int incp(const std::vector<std::string> &args);
    static int outcp(const std::vector<std::string> &args);
    static int load(const std::vector<std::string> &args);
    static int format(const std::vector<std::string> &args);
    static int defrag(const std::vector<std::string> &args);
};
