#include "Util.h"
#include <unistd.h>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <ostream>
#include <iostream>
#include <experimental/filesystem>

using namespace std;

constexpr int MAX_SIZE = 64;
constexpr int BUFFER_SIZE = 1024;
std::vector<std::string> program_list;
string get_username() {
	return string(getenv("USER"));
}

string get_hostname() {
	char buffer[BUFFER_SIZE];
	gethostname(buffer, BUFFER_SIZE);
	return string(buffer);
}

string get_path() {
	char buffer[BUFFER_SIZE];
	getcwd(buffer, BUFFER_SIZE);
	return string(buffer);
}

void set_path(const std::string& path) {
	int ret = chdir(path.c_str());
	if (ret == -1) {
		switch (errno) {
		case ENOENT:
			throw std::runtime_error("Dir doesn't exist.");
		case EACCES:
			throw std::runtime_error("Permission denied.");
		default:
			throw std::runtime_error("Unknown error.");
		}
	}
}

void execute(const vector<std::string> command) {
	char* argv[MAX_SIZE];
	char buffer[BUFFER_SIZE];
	for (size_t i = 0, index = 0; i < command.size(); ++i) {
		argv[i] = buffer + index;
		strcpy(argv[i], command[i].c_str());
		index += command[i].length() + 1;
	}
	argv[command.size()] = nullptr;

	int ret = execvp(command[0].c_str(), argv);
	if(ret==-1) {
		switch (errno) {
		case ENOENT:
			std::cout << "No such file or directory." << std::endl;
			break;
		default:
			std::cout << "Unknown error." << std::endl;
		}
	}
	exit(-1);
}


namespace fs = std::experimental::filesystem;
void initialize_program_list(){
    program_list.clear();
    std::string path = "/usr/bin/";
    for (const auto& entry : fs::directory_iterator(path))
        program_list.push_back(entry.path().filename().string());
    std::sort(program_list.begin(),program_list.end());
}
