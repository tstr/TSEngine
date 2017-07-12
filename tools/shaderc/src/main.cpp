/*
	Shader compiler tool

	usage:
		
		shaderc -t manifest.shm -o shaders/bin/ -s shaders/

		-t : target manifest, specifies list of shaders to compile and how to compile them
		-s : location of shader source files
		-o : output directory for compiled shaders


	directory structure:
		
		shaders/
			bin/
				cache/
					00001/
						28f721f7391ae39.cache
						...

					00002/
						28f721f7391ae39.cache
						...

				shader.tsh
				...
					
			src/
				shader.vs
				shader.ps
				...

			manifest.shm


	manifest.shm example:
		
		shader Shader
		{
			stage vertex
			{
				file = "shader.vs";
			}

			stage pixel
			{
				file = "shader.ps";
			}
		}
*/

/////////////////////////////////////////////////////////////////////////////////////////////////

#include "compiler.h"
#include "frontend/parser.h"

#include <tscore/path.h>
#include <tscore/pathutil.h>

#include <iostream>

/////////////////////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace ts;

bool parseCLIargs(int argc, char** argv, Path& manifest, Path& outputDir, Path& sourceDir);

int main(int argc, char** argv)
{
	Path manifestPath;
	Path outputDir;
	Path sourceDir;

	//Get CLI parameters
	if (!parseCLIargs(argc, argv, manifestPath, outputDir, sourceDir))
	{
		return EXIT_FAILURE;
	}

	if (!isFile(manifestPath))
	{
		cerr << "Shader manifest file does not exist: \"" << manifestPath.str() << "\"\n";
		return EXIT_FAILURE;
	}

	if (!isDirectory(sourceDir))
	{
		cerr << "Shader source directory does not exist: \"" << sourceDir.str() << "\"\n";
		return EXIT_FAILURE;
	}

	// existence of output directory does not neeed to be validated
	// it can be created on the fly

	//Parse manifest
	CShaderInfoParser parser(manifestPath);

	if (!parser)
	{
		cerr << "Failed to create CShaderInfoParser\n";
		return EXIT_FAILURE;
	}

	CShaderCompileEngine compileEngine(
		sourceDir,
		outputDir
	);

	//Compile shaders listed in manifest
	for (uint32 i = 0; i < parser.getShaderCount(); i++)
	{
		SShaderInfo inf;
		string name;

		parser.getShaderInfo(i, name, inf);

		if (int err = compileEngine.compileShader(name.c_str(), inf))
		{
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

bool parseCLIargs(int argc, char** argv, Path& manifest, Path& outputDir, Path& sourceDir)
{
	char curArg = 0;

	for (int i = 1; i < argc; i++)
	{
		string arg(trim(argv[i]));

		if (arg[0] == '-')
		{
			curArg = arg[1];
		}
		else
		{
			switch (curArg)
			{
			case 't':
			{
				manifest = arg;
				curArg = 0;
				break;
			}
			case 'o':
			{
				outputDir = arg;
				curArg = 0;
				break;
			}
			case 's':
			{
				sourceDir = arg;
				curArg = 0;
				break;
			}
			default:
			{
				cerr << "Invalid argument specified: " << arg << "\n";
			}
			};
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
