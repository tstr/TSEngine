/*
	Shader Compiler Class

	source
*/

#include "compiler.h"
#include "preprocessor.h"
#include <tscore/filesystem/pathhelpers.h>
#include <tsgraphics/api/renderapi.h>

//Hash function
#include "crypto/md5.h"

//Shader backends
#include "backend/hlsl.h"

#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;
using namespace ts;

#pragma pack(push, 1)
struct MD5Hash
{
	uint64 a = 0;
	uint64 b = 0;
};

struct SShaderObjectHeader
{
	byte tag0 = 'T';
	byte tag1 = 'S';
	byte tag2 = 'H';
	byte tag3 = 'O';

	MD5Hash stageVertex;
	MD5Hash stageHull;
	MD5Hash stageDomain;
	MD5Hash stageGeometry;
	MD5Hash stagePixel;
};
#pragma pack(pop)

/*
void WriteMD5Hash(ostream& output, MD5Hash hash)
{
	char hashStr[33] = { 0 };
	snprintf(hashStr, 32, "%llx%llx", hash.a, hash.b);
	output << hashStr;
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CShaderCompileEngine::Impl
{
	Path m_sourceDir;
	Path m_outputDir;

	//Compiler backends
	HLSLCompiler m_hlsl;

	int processStage(const SShaderStageInfo& shader, EShaderStage stage, MD5Hash& shaderHash)
	{
		if ((string)shader.sourceFile.str() == "")
		{
			return 0; //return ok if the filepath is purposely left empty
		}

		shaderHash = MD5Hash();

		//Resolve location
		Path src = m_sourceDir;
		src.addDirectories(shader.sourceFile);

		//Validate existence of file
		if (!isFile(src))
		{
			cerr << "File does not exist: " << src.str() << endl;
			return -1;
		}

		stringstream srcStream;
		
		//Preprocess the file
		if (EPreprocessorStatus e = preprocessFile(src, srcStream, shader.macros, (uint)shader.macroCount, &m_sourceDir, 1))
		{
			cerr << "Preprocessor error\n";
			return -1;
		}
		
		//Create hash of preprocessed file
		{
			MD5_CTX ctx;
			md5_init(&ctx);
			md5_update(&ctx, (const BYTE*)srcStream.str().c_str(), srcStream.str().size());
			md5_final(&ctx, (BYTE*)&shaderHash);
		}

		//Compile with hlsl backend
		return compileStage(m_hlsl, shaderHash, srcStream.str().c_str(), shader.entryPoint, stage);
	}

private:

	int compileStage(IShaderBackend& backend, MD5Hash hash, const char* code, const char* entrypoint, EShaderStage stage)
	{
		//Format 128bit hash
		char hashStr[33] = { 0 };
		snprintf(hashStr, 32, "%llx%llx", hash.a, hash.b);

		MemoryBuffer bytecode;
		if (!backend.compile(code, bytecode, entrypoint, stage))
		{
			return -1;
		}

		char idStr[9] = { 0 };
		snprintf(idStr, 8, "%x", backend.getId());

		Path cacheFile = m_outputDir;
		cacheFile.addDirectories("cache");
		cacheFile.addDirectories(idStr);
		cacheFile.addDirectories(hashStr);

		fstream f(createFile(cacheFile, ios::binary | ios::out));
		f.write((const char*)bytecode.pointer(), bytecode.size());

		if (f.fail())
		{
			return -1;
		}

		return 0;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CShaderCompileEngine::CShaderCompileEngine(
	const Path& srcdir,
	const Path& outputdir
) : pImpl(new Impl())
{
	pImpl->m_outputDir = outputdir;
	pImpl->m_sourceDir = srcdir;
}

CShaderCompileEngine::~CShaderCompileEngine()
{
	if (pImpl)
	{
		delete pImpl;
		pImpl = nullptr;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CHECK(x) if (x) status |= 1

int CShaderCompileEngine::compileShader(const char* name, const SShaderInfo& stage)
{
	int status = 0;
	int ret = false;

	Path shaderObjectName(pImpl->m_outputDir);
	shaderObjectName.addDirectories((string)name + ".tsh");
	fstream shaderFile(createFile(shaderObjectName, ios::out | ios::binary));

	if (shaderFile.fail())
	{
		return -1;
	}

	SShaderObjectHeader header;

	ret = pImpl->processStage(stage.vertexStage, eShaderStageVertex, header.stageVertex);
	CHECK(ret);

	ret = pImpl->processStage(stage.hullStage, eShaderStageHull, header.stageHull);
	CHECK(ret);

	ret = pImpl->processStage(stage.domainStage, eShaderStageDomain, header.stageDomain);
	CHECK(ret);

	ret = pImpl->processStage(stage.geometryStage, eShaderStageGeometry, header.stageGeometry);
	CHECK(ret);

	ret = pImpl->processStage(stage.pixelStage, eShaderStagePixel, header.stagePixel);
	CHECK(ret);

	shaderFile.write((const char*)&header, sizeof(SShaderObjectHeader));
	if (shaderFile.fail())
	{
		return -1;
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
