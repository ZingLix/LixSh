#include "Shell.h"
#include "Util.h"
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <readline/readline.h>
#include <readline/history.h>
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
    while (status_) {
        print_info();
        char* line = readline(prefix_.c_str());
       // getline(cin, str, '\n');
        string str(line);
        add_history(line);
        free(line);
        auto [jobs, bg] = parse(str);
        if (jobs.size() == 0) { cout << endl; continue; }
        builtin_cmd type = builtin_type(jobs[0]->program());
        if (type != builtin_cmd::none) {
            run_builtin(type, jobs[0]->commands());
        } else {
            pid_t pid;
            if (bg) cout << "Running in background: " << endl;
            for (auto& j : jobs) {
                pid = j->run();
                job_map_[pid] = move(j);
                job_map_.insert(make_pair(pid, move(j)));
                if (bg) {
                    job_map_[pid]->go_background();
                    cout << "\t[pid] " << pid << "\t" << job_map_[pid]->raw_command() << endl;
                }
            }
            int status;
            if (!bg) {
                waitpid(pid, &status, 0);
                child_terminated(pid);
            }
        }

        cout << endl;
    }
}

char* command_generator(const char* text,int state){
    static int list_index;
	string src(text);
    char *name;

    if (state==0){
      list_index = 0;

	auto it = program_list.begin();
  	while (true)
    {
		if((*it)[0]==src[0]&& it->find(src)==0) {
			list_index=it-program_list.begin();

		return strdup(it->c_str());
		}
++it;
    }
}else{
	auto it = program_list.begin()+list_index+state;
	if(it->find(src)==0){
		return strdup(it->c_str());
	}
}
  /* If no names matched, then return NULL. */
  return ((char *)NULL);
}

char** completer(const char*text,int start,int end){
	char **matches=nullptr;
	if(start==0)
		matches=rl_completion_matches(text,command_generator);

	return matches;
}

void Shell::init() {
    if (current_shell != nullptr) {
        throw std::runtime_error("Another shell is running");
    }
    current_shell = this;
    status_ = true;
    username_ = get_username();
    hostname_ = get_hostname();
    prefix_ = username_ == "root" ? "# " : "$ ";
    cur_path_ = get_path();
    signal_init();
    initialize_program_list();
	 rl_attempted_completion_function = completer;
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

std::pair<std::vector<std::unique_ptr<Job>>, bool> Shell::parse(const std::string& str) {
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
    if (count(lastone.begin(), lastone.end(), ' ') != static_cast<int> (lastone.length()))
        list.push_back(move(lastone));
    if (bg && list.back() != "&") throw std::logic_error("Invalid input.");
    if (list.size() == 0) return{ std::vector<std::unique_ptr<Job>>(),false };
    int pip[2];
    bool valid = false;
    std::vector<std::unique_ptr<Job>> jobs;
    for (auto it = list.begin(); it != list.end(); ++it) {
        if (*it == ">") {
            if (jobs.size() == 0)throw std::logic_error("Invalid input.");
            jobs.back()->redirect_output_to_file(trim(*++it));
        } else if (*it == "<") {
            if (jobs.size() == 0)throw std::logic_error("Invalid input.");
            jobs.back()->redirect_input_to_file(trim(*++it));
        } else if (*it == "|") {
            pipe(pip);
            valid = true;
            jobs.back()->redirect_output_to_pipe(pip);
        } else if (*it == "&") {} else {
            jobs.push_back(make_unique<Job>(*it));
            if (valid) {
                valid = false;
                jobs.back()->redirect_input_to_pipe(pip);
            }
        }
    }
    return { move(jobs),bg };
}

void Shell::child_terminated(pid_t pid) {
    if (job_map_[pid]->is_background()) {
        cout << "[pid] " << pid << "\tterminated\t" << job_map_[pid]->raw_command() << endl;
    }
    job_map_.erase(job_map_.find(pid));
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

