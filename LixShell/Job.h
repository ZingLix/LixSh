#pragma once
#include <string>
#include "Util.h"
#include <unistd.h>
#include <stdexcept>
#include <fcntl.h>
#include <cstdlib>
#include <iostream>

class Job
{
public:
	Job(const std::string& cmd)
        :command(split_str(cmd)),
        status_(Status::foreground),
        pipe_in{-1,-1},
        pipe_out{-1,-1}
        
    {
	}

    ~Job() {
        if (pipe_in[0] != -1) close(pipe_in[0]);
        if (pipe_out[1] != -1) close(pipe_out[1]);
	}

	void redirect_output_to_file(const std::string& filename) {
		output = RedirectType::FILE;
		output_filename = filename;
	}

    void redirect_input_to_file(const std::string& filename) {
        input = RedirectType::FILE;
        input_filename = filename;
    }

    void redirect_input_to_pipe(int fd[2]) {
        pipe_in[0] = fd[0];
        pipe_in[1] = fd[1];
    }

    void redirect_output_to_pipe(int fd[2]) {
        pipe_out[0] = fd[0];
        pipe_out[1] = fd[1];
	}

	pid_t run() {
		pid_t pid = fork();
		if(pid<0) {
			throw std::runtime_error("errno: " + std::to_string(errno));
		}else if(pid==0) {
			if(output==RedirectType::FILE) {
				int output_fd = open(output_filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR, S_IWUSR);
				dup2(output_fd, STDOUT_FILENO);
				dup2(output_fd, STDERR_FILENO);
			}
            if (input == RedirectType::FILE) {
                int input_fd = open(input_filename.c_str(), O_RDONLY);
                if(input_fd==-1) {
                    std::cout << errno << std::endl;
                }
                dup2(input_fd, STDIN_FILENO);
            }
            if(pipe_in[0]!=-1) {
                close(pipe_in[1]);
                dup2(pipe_in[0], STDIN_FILENO);
            }
            if(pipe_out[1]!=-1) {
                close(pipe_out[0]);
                dup2(pipe_out[1], STDOUT_FILENO);
                dup2(pipe_out[1], STDERR_FILENO);
            }
			execute(command);
		}
		return pid;
	}

    std::string program() {
        return command[0];
	}

    argv_t commands() {
        return command;
	}

    std::string raw_command() {
        std::string res;
        for(auto& s:command) {
            res += s + " ";
        }
        return res;
	}

    void go_background() {
        status_ = Status::background;
	}

    bool is_background() {
        return status_ == Status::background;
	}

private:
	enum class RedirectType
	{
		STD,FILE,PIPE
	};

    enum class Status
    {
        foreground,background
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
        for(auto&s:list) {
            if(s[0]=='$'&&s.length()>1) {
                s = get_environment(std::string(s.begin() + 1, s.end()));
            }
            if(s[0]=='~') {
                s = get_environment("HOME") + std::string(s.begin() + 1, s.end());
            }
        }
		return list;
	}

	argv_t command;
    Status status_;
    int pipe_in[2], pipe_out[2];
	RedirectType output;
	RedirectType input;
	std::string output_filename;
	std::string input_filename;
};
