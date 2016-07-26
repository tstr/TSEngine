

#include "application.h"
#include <string>


int main(int argc, char** argv)
{
	std::string cmdargs;
	
	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			cmdargs += argv[i];
			cmdargs += " ";
		}
	}
	
	ts::Application app(cmdargs.c_str());
};