#include "Util.h"
#include <unistd.h>
#include <cstdlib>
#include <string>

using namespace std;

constexpr int BUFFER_SIZE = 50;

string get_username()
{
	return string(getenv("USER"));
}

string get_hostname()
{
	char buffer[BUFFER_SIZE];
	gethostname(buffer, BUFFER_SIZE);
	return string(buffer);
}