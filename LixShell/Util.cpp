#include "Util.h"
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <vector>
#include <cstring>
#include <stdexcept>

using namespace std;

constexpr int MAX_SIZE = 64;
constexpr int BUFFER_SIZE = 1024;

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

	execvp(("/bin/" + command[0]).c_str(), argv);
	exit(0);

}
