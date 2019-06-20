#include "Shell.h"

int main(int argc, char* argv[], char* envp[])
{
	Shell sh;
	sh.loop();
	return 0;
}
