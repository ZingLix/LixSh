#include "Shell.h"
#include "Util.h"
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <string>
using namespace std;

Shell::Shell()
	:cur_path_("~") {
	init();
}

Shell::Shell(const char* path)
	: cur_path_(path) {
	init();
}


void Shell::loop() {
	string str;
	while (status_) {
		print_info();
		cout << prefix_ << " ";
		getline(cin,str,'\n');
		auto vec = split_str(str);
		char* const argv[] = { "ls","-a",NULL };
		pid_t pid;
		if ((pid = fork()) == 0) {
			execvp("/bin/ls", argv);
		}
		waitpid(pid, NULL, 0);
		cout << endl;
	}
}

void Shell::init() {
	status_ = true;
	username_ = get_username();
	hostname_ = get_hostname();
	prefix_ = username_ == "root" ? '#' : '$';
}


void Shell::print_info() {
	cout << "# " << username_ << " @ " << hostname_ << " : " << cur_path_ << endl;
}

std::vector<std::string> Shell::split_str(const std::string& str) {
	std::vector<std::string> list;
	auto last_it = str.begin();
	bool is_space = str[0] == ' ';
	for (auto it = str.begin(); it != str.end(); ++it) {
		if (*it == ' '&&!is_space) {
			list.push_back(string(last_it, it));
			is_space = true;
		}
		if(*it!=' '&&is_space) {
			is_space = false;
			last_it = it;
		}
	}
	if(str.back()!=' ')
		list.push_back(string(last_it, str.end()));
	return list;
}


