#pragma once
#include <string>
#include "Util.h"
#include <unistd.h>
#include <stdexcept>
#include <fcntl.h>
#include <cstdlib>

class Job
{
public:
	Job(const std::string& cmd):command(split_str(cmd)) {
		
	}

	void redirect_output_to_file(const std::string& filename) {
		output = RedirectType::FILE;
		output_filename = filename;
	}

	pid_t run() {
		pid_t pid = fork();
		if(pid<0) {
			throw std::runtime_error("errno: " + std::to_string(errno));
		}else if(pid==0) {
			if(output==RedirectType::FILE) {
				int output_fd = open(output_filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC);
				dup2(output_fd, STDOUT_FILENO);
				dup2(output_fd, STDERR_FILENO);
			}
			execute(command);
		}
		return pid;
	}

private:
	enum class RedirectType
	{
		STD,FILE,PIPE
	};

	argv_t split_str(const std::string& str) {
		std::vector<std::string> list;
		auto last_it = str.begin();
		bool is_space = str[0] == ' ';
		for (auto it = str.begin(); it != str.end(); ++it) {
			if (*it == ' ' && !is_space) {
				list.push_back(std::string(last_it, it));
				is_space = true;
			}
			if (*it != ' ' && is_space) {
				is_space = false;
				last_it = it;
			}
		}
		if (str.back() != ' ')
			list.push_back(std::string(last_it, str.end()));
		return list;
	}

	argv_t command;
	RedirectType output;
	RedirectType input;
	std::string output_filename;
	std::string input_filename;
};
