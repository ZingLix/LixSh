#include "Shell.h"
#include "Util.h"
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <string>
using namespace std;

Shell* current_shell=nullptr;

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
        bool bg=false;
		print_info();
		cout << prefix_ << " ";
		getline(cin,str,'\n');
		auto vec = split_str(str);
        if(vec.back()=="&") {
            bg=true;
            vec.pop_back();
        }
        builtin_cmd type=builtin_type(vec[0]);
        if(type!=builtin_cmd::none){
            run_builtin(type,vec);
        }
		auto pid = execute(vec);
        if(bg) cout << "Running in background: [pid]"<<pid<<endl;
        else waitpid(pid, NULL, 0);
		cout << endl;
	}
}

void Shell::init() {
    if(current_shell!=nullptr){
        throw std::runtime_error("Another shell is running");
    }
    current_shell=this;
	status_ = true;
	username_ = get_username();
	hostname_ = get_hostname();
	prefix_ = username_ == "root" ? '#' : '$';
    cur_path_= get_path();
    signal_init();
}

void sigchld_handler(int sig){
    (void)sig;
    int status;
    while(true){
        pid_t pid = waitpid(-1,&status,WNOHANG);
        if(pid<=0) break;
        current_shell->child_terminated(pid);
    }
}

void Shell::signal_init(){
    signal(SIGCHLD,sigchld_handler);
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

void Shell::child_terminated(pid_t pid){
    cout<<"Process terminated: [pid]"<<pid<<endl;
}

Shell::builtin_cmd Shell::builtin_type(const std::string& str){
    if(str=="cd"){
        return builtin_cmd::cd;
    }
    return builtin_cmd::none;
}

void Shell::run_builtin(Shell::builtin_cmd cmd, const argv_t& argv){
    if(cmd==builtin_cmd::cd){
        builtin_cd(argv);
    }
}

void Shell::builtin_cd(const argv_t& argv){
    const string& newpath=argv[1];
    try{
    if(newpath[0]=='/'){
        set_path(newpath);
    }else if(newpath==".."){
        auto it=cur_path_.end()-1;
        for(;it!=cur_path_.begin();--it){
            if(*it=='/') break;
        }
        if(it==cur_path_.begin()) ++it;
        set_path(string(cur_path_.begin(),it));
    }else{
        set_path(cur_path_+"/"+newpath);
    }
    cur_path_=get_path();
    }catch(const std::exception& e){
        cout<<e.what()<<endl;
    }
}
