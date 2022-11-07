#pragma once

#include <memory>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "pseudofat.h"

typedef void (*command)(const std::unique_ptr<PseudoFS> &, const std::vector<std::string> &);
typedef std::map<std::string, command> command_map;

void help(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &tokens);
void cp(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &tokens);
void mv(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
void rm(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
void mkdir(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
void rmdir(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
void ls(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
void cat(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
void cd(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
void pwd(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
void info(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
void incp(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
void outcp(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
void load(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
void format(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
void defrag(const std::unique_ptr<PseudoFS> &fs, const std::vector<std::string> &args);
