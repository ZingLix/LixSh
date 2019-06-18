#pragma once
#include <string>
#include <vector>
#include <sys/types.h>

std::string get_username();
std::string get_hostname();

std::string get_path();
void set_path(const std::string& path);

pid_t execute(const std::vector<std::string> command);
