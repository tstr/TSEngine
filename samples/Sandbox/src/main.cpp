/*
	Program entry point
*/

#include "Sandbox.h"

using namespace ts;

int main(int argc, char** argv)
{
	Sandbox app(argc, argv);
	return app.start();
}
