/*
	Resource Generator:

	Takes a set of Resource Schema files describing the layout of binary resources
	and generates a corresponding set of code files which can create and modify these resources.

	Usage:

	rcschema [OPTIONS] [FILES...]

	rcschema --out /output_dir/ file0.schema file1.schema ...
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
	String outDirectory = ".";
	bool showHelp = false;
	bool noLoader = false;
	bool noBuilder = false;
	//List of resource schema files
	ArgumentReader::ParameterList fileList;

	ArgumentReader cliArgs;
	cliArgs.addParameter("out", outDirectory, "Output directory for generated files");
	cliArgs.addOption("help", showHelp, "Show help information");
	cliArgs.addOption("noloader", noLoader, "Do not generate a loader class");
	cliArgs.addOption("nobuilder", noBuilder, "Do not generate a builder class");
	cliArgs.setUnused(fileList);

	//Parse command line arguments
	if (cliArgs.parse(argc, argv))
	{
		cout << "Usage:\n";
		cerr << "rcschema [OPTIONS] [FILES...]\n";
		cliArgs.print(cerr);
		return CLI_EXIT_INVALID_ARGUMENT;
	}

	//If --help was specified print parameters
	if (showHelp)
	{
		cout << "Usage:\n";
		cout << "rcschema [OPTIONS] [FILES...]\n";
		cliArgs.print(cout);
		return CLI_EXIT_SUCCESS;
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
			//Set generator flags
			uint32 flags = GENERATE_ALL;
			if (noBuilder) flags &= ~GENERATE_BUILDER;
			if (noLoader) flags &= ~GENERATE_LOADER;

			//Debug print
			cout << fileName << " -> " << schema.getName() << ".rcs.h\n";

			//Run C++ generator
			if (!CPPGenerator().generate(schema, outPath, flags))
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
