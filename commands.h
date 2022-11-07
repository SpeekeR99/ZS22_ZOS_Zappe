#pragma once

#include <memory>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "pseudofat.h"

typedef int (*command)(const std::unique_ptr<PseudoFS> &, const std::vector<std::string> &);
typedef std::map<std::string, command> command_map;

class Commands {
public:
    static int help(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
    static int cp(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
    static int mv(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
    static int rm(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
    static int mkdir(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
    static int rmdir(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
    static int ls(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
    static int cat(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
    static int cd(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
    static int pwd(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
    static int info(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
    static int incp(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
    static int outcp(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
    static int load(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
    static int format(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
    static int defrag(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
};
