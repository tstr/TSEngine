/*
	Shader source file class
*/

#pragma once

#include <sstream>
#include <unordered_set>

#include "ShaderCompiler.h"
#include "ShaderParser.h"
#include "Preprocessor.h"

namespace ts
{
	class ShaderSource
	{
	public:

		enum Error
		{
			OK,
			PREPROCESSOR_ERROR,
			FILE_NOT_FOUND_ERROR,
			COMPILER_ERROR,
			METAPARSER_ERROR,
		};

		ShaderSource(const ShaderSource&) = delete;

		ShaderSource(const std::string& sourceFilePath) { load(sourceFilePath); }

		/*
			Read and parse the given shader source
		*/
		Error load(const std::string& sourceFilePath);

		/*
			Compile the given shader parser
		*/
		Error compile(const std::string& outputDir);

		/*
			Source info getters
		*/
		const ShaderParser& meta() const { return m_metadata; }
		const std::unordered_set<std::string>& dependencyNames() const { return m_dependencies; }
		const std::string& errorString() const { return m_errorString; }
		Error errorCode() const { return m_errorCode; }

		const std::string& sourceName() const { return m_shaderSourcePath; }
		std::string sourceText() const { return m_shaderSource.str(); }

	private:

		//Include file dependencies
		std::unordered_set<std::string> m_dependencies;

		//Shader metadata parser
		ShaderParser m_metadata;
		//Compiler
		ShaderCompiler m_compileEngine;
		//Last error
		std::string m_errorString;
		Error m_errorCode;

		//Shader source
		std::string m_shaderSourcePath;
		std::stringstream m_shaderSource;

		//Set error state
		Error setError(Error code, const std::string& message)
		{
			m_errorCode = code;
			m_errorString = message;
			return m_errorCode;
		}
	};
}
