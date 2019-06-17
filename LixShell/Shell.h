#pragma once
#include <string>
#include <vector>

class Shell
{
public:
	Shell();
	Shell(const char* path);
	void loop();

private:
	void init();
	void print_info();
	std::vector<std::string> split_str(const std::string& str);

	bool status_;
	std::string username_;
	std::string hostname_;
	char prefix_;
	std::string cur_path_;
};
