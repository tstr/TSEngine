/*
	Shader source file
*/

#include <tscore/path.h>
#include <tscore/pathutil.h>

#include "ShaderSource.h"

using namespace ts;
using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderSource::Error ShaderSource::load(const string& sourceFilePath)
{
	m_shaderSourcePath = sourceFilePath;

	if (!isFile(m_shaderSourcePath))
	{
		return setError(Error::FILE_NOT_FOUND_ERROR, format("Source file not found \"%\"", sourceFilePath));
	}

	//Reset everything
	m_dependencies.clear();
	m_errorString.clear();
	m_shaderSource.str("");
	m_shaderSource.clear();

	//Preprocessing step
	CPreprocessor pp;
	pp.setCommentStrip(true);

	//Strip comments and resolve preprocessor directives
	if (EPreprocessorStatus ppStatus = pp.process(m_shaderSourcePath, m_shaderSource))
	{
		return setError(Error::PREPROCESSOR_ERROR, format("ERROR: unable to preprocess source file (%)", ppStatus));
	}

	for (const Path& p : pp.getIncludeDependencies())
		m_dependencies.insert(p.str());

	//Reset read
	m_shaderSource.seekg(0);

	try
	{
		//Parsing step - extract reflection info from shader source
		m_metadata.parse(m_shaderSource);
	}
	catch (std::exception& e)
	{
		return setError(Error::METAPARSER_ERROR, (string)"ERROR: Unable to parse metadata from source file\n" + e.what());
	}

	//Reset read
	m_shaderSource.seekg(0);

	return setError(Error::OK, "");
}

ShaderSource::Error ShaderSource::compile(const std::string& outputDir)
{
	//Do not proceed if an error has occured
	if (m_errorCode != OK)
	{
		return m_errorCode;
	}

	ShaderCompilerOptions opt;
	opt.vsEntry = (m_metadata.isFunction("VS")) ? "VS" : "";
	opt.tcsEntry = (m_metadata.isFunction("TC")) ? "TC" : "";
	opt.tesEntry = (m_metadata.isFunction("TE")) ? "TE" : "";
	opt.gsEntry = (m_metadata.isFunction("GS")) ? "GS" : "";
	opt.psEntry = (m_metadata.isFunction("PS")) ? "PS" : "";

	//Resolve object filename
	string objectFileName = m_shaderSourcePath;
	objectFileName = objectFileName.substr(0, objectFileName.find_last_of('.'));
	objectFileName += ".shader";
	objectFileName = Path(objectFileName).getDirectoryTop().str();

	//Resolve path
	Path objectPath = outputDir;
	objectPath.addDirectories(objectFileName);

	//Create shader object file
	fstream shaderObjectFile(createFile(objectPath, ios::binary | ios::ate | ios::out));

	if (shaderObjectFile.fail())
	{
		return setError(Error::COMPILER_ERROR, format("ERROR: Unable to create shader object file \"%\"", objectPath.str()));
	}

	m_shaderSource.seekg(0);
	m_shaderSource.seekp(0);

	//Compilation step
	if (int err = m_compileEngine.compile(m_shaderSource, shaderObjectFile, opt))
	{
		return setError(Error::COMPILER_ERROR, (string)"ERROR: Unable to compile source file file:\n" + m_compileEngine.lastError());	
	}

	//Force writes to file just in case
	shaderObjectFile.flush();

	return setError(Error::OK, "");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
