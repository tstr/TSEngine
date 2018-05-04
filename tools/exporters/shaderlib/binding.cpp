/*
    ShaderLib python bindings
*/

#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>

#include "ShaderCompiler.h"
#include "ShaderParser.h"
#include "Preprocessor.h"

namespace py = pybind11;

using namespace std;
using namespace ts;

/*
	Compile a list of shader sources and store the resulting objects in the given output directory
*/
bool compile(const std::vector<std::string>& shaderFileNames, const std::string& outputDirName)
{
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
				return false;
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
					objectFileName += ".shader";
					objectFileName = Path(objectFileName).getDirectoryTop().str();

					//Resolve path
					Path objectPath = outputDirName;
					objectPath.addDirectories(objectFileName);

					//Create shader object file
					fstream shaderObjectFile(createFile(objectPath, ios::binary | ios::ate | ios::out));

					if (shaderObjectFile.fail())
					{
						cerr << "Unable to create shader object file \"" << objectPath.str() << "\"\n";
						return false;
					}

					//Compilation step
					if (int err = compileEngine.compile(shaderSource, shaderObjectFile, opt))
					{
						cerr << "Unable to compile source file file \"" << shaderPath.str() << "\"\n";
						return false;
					}

					//Force writes to file just in case
					shaderObjectFile.flush();
				}
				else
				{
					cerr << "Unable to parse metadata from source file \"" << shaderPath.str() << "\"\n";
					return false;
				}
			}
		}
		else
		{
			cerr << "Unable to find source file \"" << shaderPath.str() << "\"\n";
			return false;
		}
	}

	return true;
}

PYBIND11_MODULE(shaderlib, m)
{
    m.def("compile",
		&compile,
		py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>()
	);
}
