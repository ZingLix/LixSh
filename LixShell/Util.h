#pragma once
#include <string>
#include <vector>
#include <sys/types.h>

using argv_t = std::vector<std::string>;
std::vector<std::string> program_list;
std::string get_username();
std::string get_hostname();

std::string get_path();
void set_path(const std::string& path);

void execute(const std::vector<std::string> command);
void initialize_program_list();