/*
	Resource Generator:

	Takes a set of Resource Schema files describing the layout of binary resources
	and generates a corresponding set of code files which can create and modify these resources.
*/

#include <iostream>
#include <sstream>

#include <cli/Arguments.h>
#include <cli/Constants.h>

#include "Schema.h"
#include "SchemaReader.h"

#include "generators/Cpp.h"

using namespace std;
using namespace ts;
using namespace ts::cli;
using namespace ts::rc;

int main(int argc, char** argv)
{
	//Output directory for generated files
	String outDirectory = "";
	//List of resource schema files
	ArgumentReader::ParameterList fileList;

	ArgumentReader cliArgs;
	cliArgs.addParameter("out", outDirectory);
	cliArgs.setUnused(fileList);

	//Parse command line arguments
	if (cliArgs.parse(argc, argv))
	{
		cliArgs.print(cerr);
		return CLI_EXIT_INVALID_ARGUMENT;
	}

	//Resolve output directory
	Path outPath(outDirectory);

	if (fileList.empty())
	{
		cerr << "ERROR: Expected list of files\n";
		return CLI_EXIT_INVALID_ARGUMENT;
	}
	
	for (String fileName : fileList)
	{
		Schema schema;

		//Read schema declarations
		if (SchemaReader().read(fileName, schema))
		{
			//Run C++ generator
			if (!CPPGenerator().generate(schema, outPath))
			{
				return CLI_EXIT_FAILURE;
			}
		}
		else
		{
			return CLI_EXIT_FAILURE;
		}

	}
	
	return CLI_EXIT_SUCCESS;
}
