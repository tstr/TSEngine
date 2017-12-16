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

#include <tscore/path.h>
#include <tscore/pathutil.h>

#include <iostream>

#include <cli/Arguments.h>
#include <cli/Constants.h>

#include "ShaderCompiler.h"
#include "ShaderParser.h"
#include "Preprocessor.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace ts;

int main(int argc, char** argv)
{
	//Arguments
	String outputDirName;
	vector<String> shaderFileNames;

	//Parse command line arguments
	cli::ArgumentReader args;
	args.addParameter("out", outputDirName, "Output directory for compiled shaders");
	args.setUnused(shaderFileNames);
	
	if (int err = args.parse(argc, argv))
	{
		return cli::CLI_EXIT_INVALID_ARGUMENT;
	}

	// existence of output directory does not neeed to be validated
	// it can be created on the fly
	Path outputDir = outputDirName;

	ShaderCompiler compileEngine;

	// For each shader source file
	for (const Path shaderPath : shaderFileNames)
	{
		if (isFile(shaderPath))
		{
			//Shader source stream
			stringstream shaderSource;

			//Preprocessing step
			CPreprocessor pp;
			pp.setCommentStrip(true);
			
			//Strip comments and resolve preprocessor directives
			if (EPreprocessorStatus ppStatus = pp.process(shaderPath, shaderSource))
			{
				cerr << "Error " << ppStatus << " unable to preprocess source file \"" << shaderPath.str() << "\"\n";
				return cli::CLI_EXIT_FAILURE;
			}
			else
			{
				//Parsing step - extract reflection info from shader source
				ShaderParser parser(shaderSource);

				if (parser.good())
				{
					//Reset read
					shaderSource.seekg(0);

					ShaderCompilerOptions opt;
					opt.vsEntry = (parser.isFunction("VS")) ? "VS" : "";
					opt.tcsEntry = (parser.isFunction("TC")) ? "TC" : "";
					opt.tesEntry = (parser.isFunction("TE")) ? "TE" : "";
					opt.gsEntry = (parser.isFunction("GS")) ? "GS" : "";
					opt.psEntry = (parser.isFunction("PS")) ? "PS" : "";

					//Resolve object filename
					string objectFileName = shaderPath.str();
					objectFileName = objectFileName.substr(0, objectFileName.find_last_of('.'));
					objectFileName += ".tsh";
					objectFileName = Path(objectFileName).getDirectoryTop().str();

					//Resolve path
					Path objectPath = outputDirName;
					objectPath.addDirectories(objectFileName);

					//Create shader object file
					fstream shaderObjectFile(createFile(objectPath, ios::binary | ios::ate | ios::out));

					if (shaderObjectFile.fail())
					{
						cerr << "Unable to create shader object file \"" << objectPath.str() << "\"\n";
						return cli::CLI_EXIT_FAILURE;
					}

					//Debug print
					cout << shaderPath.str() << " -> " << objectPath.str() << endl;

					//Compilation step
					if (int err = compileEngine.compile(shaderSource, shaderObjectFile, opt))
					{
						cerr << "Unable to compile source file file \"" << shaderPath.str() << "\"\n";
						return cli::CLI_EXIT_FAILURE;
					}

					//Force writes to file just in case
					shaderObjectFile.flush();
				}
				else
				{
					cerr << "Unable to parse metadata from source file \"" << shaderPath.str() << "\"\n";
					return cli::CLI_EXIT_FAILURE;
				}
			}
		}
		else
		{
			cerr << "Unable to find source file \"" << shaderPath.str() << "\"\n";
			return cli::CLI_EXIT_FAILURE;
		}
	}

	return cli::CLI_EXIT_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
