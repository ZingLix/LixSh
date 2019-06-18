#pragma once
#include <string>
#include <vector>

class Shell
{
public:
	Shell();
	Shell(const char* path);
	void loop();

    void child_terminated(pid_t pid);

private:
    using argv_t = std::vector<std::string>;

    enum class builtin_cmd{
        cd,none
    };

	void init();
    void signal_init();
	void print_info();
	std::vector<std::string> split_str(const std::string& str);
    builtin_cmd builtin_type(const std::string& str);
    void run_builtin(builtin_cmd cmd, const argv_t& argv);

    void builtin_cd(const argv_t& argv);

	bool status_;
	std::string username_;
	std::string hostname_;
	char prefix_;
	std::string cur_path_;
};
