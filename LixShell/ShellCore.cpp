#include "Shell.h"
#include "Util.h"
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <algorithm>
#include "Job.h"
using namespace std;

Shell* current_shell = nullptr;

Shell::Shell() {
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
        getline(cin, str, '\n');
        auto [jobs, bg] = parse(str);
        //      if(vec.back()=="&") {
  //          bg=true;
  //          vec.pop_back();
  //      }
        builtin_cmd type=builtin_type(jobs[0].program());
        if(type!=builtin_cmd::none){
            run_builtin(type,jobs[0].commands());
        }else {
            auto pid = jobs[0].run();
            int status;
            if (bg) cout << "Running in background: [pid]" << pid << endl;
            else {
                waitpid(pid, &status, 0);
                //		cout << status<<endl;
            }
        }
        
        cout << endl;
    }
}

void Shell::init() {
    if (current_shell != nullptr) {
        throw std::runtime_error("Another shell is running");
    }
    current_shell = this;
    status_ = true;
    username_ = get_username();
    hostname_ = get_hostname();
    prefix_ = username_ == "root" ? '#' : '$';
    cur_path_ = get_path();
    signal_init();
}

void sigchld_handler(int sig) {
    (void)sig;
    int status;
    while (true) {
        pid_t pid = waitpid(-1, &status, WNOHANG);
        if (pid <= 0) break;
        current_shell->child_terminated(pid);
    }
}

void Shell::signal_init() {
    signal(SIGCHLD, sigchld_handler);
}

void Shell::print_info() {
    cout << "# " << username_ << " @ " << hostname_ << " : " << cur_path_ << endl;
}

std::pair<std::vector<Job>, bool> Shell::parse(const std::string& str) {
    std::vector<std::string> list;
    auto trim = [](const std::string& str) {
        auto first = str.find_first_not_of(" ");
        auto last = str.find_last_not_of(" ");
        return string(str.begin() + first, str.begin() + last + 1);
    };
    auto last_it = str.begin();
    auto bg = false;
    for (auto it = str.begin(); it != str.end(); ++it) {
        if (*it == '|' || *it == '>' || *it == '<' || *it == '&') {
            list.emplace_back(last_it, it);
            list.emplace_back(string(1, *it));
            last_it = it + 1;
            if (*it == '&') bg = true;
        }
    }
    std::string lastone = std::string(last_it, str.end());
    if (count(lastone.begin(), lastone.end(), ' ') != lastone.length())
        list.push_back(move(lastone));
    if (bg && list.back() != "&") throw std::logic_error("Invalid input.");
    std::vector<Job> jobs;
    for (auto it = list.begin(); it != list.end(); ++it) {
        if (*it == ">") {
            if (jobs.size() == 0)throw std::logic_error("Invalid input.");
            jobs.back().redirect_output_to_file(trim(*++it));
        } else if (*it == "<") {
            if (jobs.size() == 0)throw std::logic_error("Invalid input.");
            jobs.back().redirect_input_to_file(trim(*++it));
        } else {
            jobs.emplace_back(*it);
        }
    }
    return { jobs,bg };
}

void Shell::child_terminated(pid_t pid) {
    cout << "Process terminated: [pid]" << pid << endl;
}

Shell::builtin_cmd Shell::builtin_type(const std::string& str) {
    if (str == "cd") {
        return builtin_cmd::cd;
    }
    return builtin_cmd::none;
}

void Shell::run_builtin(Shell::builtin_cmd cmd, const argv_t& argv) {
    if (cmd == builtin_cmd::cd) {
        builtin_cd(argv);
    }
}

void Shell::builtin_cd(const argv_t& argv) {
    const string& newpath = argv[1];
    try {
        if (newpath[0] == '/') {
            set_path(newpath);
        } else if (newpath == "..") {
            auto it = cur_path_.end() - 1;
            for (; it != cur_path_.begin(); --it) {
                if (*it == '/') break;
            }
            if (it == cur_path_.begin()) ++it;
            set_path(string(cur_path_.begin(), it));
        } else {
            set_path(cur_path_ + "/" + newpath);
        }
        cur_path_ = get_path();
    } catch (const std::exception& e) {
        cout << e.what() << endl;
    }
}
