/*
	Program entry point
*/

#include "Sandbox.h"

using namespace ts;

int main(int argc, char** argv)
{
	CEngineEnv env(argc, argv);

	return env.start(Sandbox(env));
}
