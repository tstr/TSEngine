/*
	Shader Compiler Class

	source
*/

#include "ShaderCompiler.h"
#include "Preprocessor.h"
#include <tscore/pathutil.h>

//Hash function
#include "crypto/sha256.h"

//Shader backends
#include "backend/hlsl.h"

//Shader resource
#include <tsgraphics/schemas/Shader.rcs.h>

#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;
using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32 formatSignature(char a, char b, char c, char d)
{
	return (d << 24) | (c << 16) | (b << 8) | (a << 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Compiler implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct ShaderCompiler::Impl
{
	//Compiler backends
	HLSLCompiler m_hlsl;

	bool processStage(tsr::ShaderStageBuilder& stageRsc, const String& source, const String& entryPoint, EShaderStage stage)
	{
		//Create hash of preprocessed file
		/*{
			tsr::ShaderHash hash;

			SHA256_CTX ctx;
			sha256_init(&ctx);
			sha256_update(&ctx, (const BYTE*)shader.entryPoint.c_str(), shader.entryPoint.size());
			sha256_update(&ctx, (const BYTE*)srcStream.str().c_str(), srcStream.str().size());
			sha256_final(&ctx, (BYTE*)&hash);
		}*/

		switch (stage)
		{
		case EShaderStage::eShaderStageVertex:
			stageRsc.set_signature(formatSignature('S','V','T','X'));
			break;
		case EShaderStage::eShaderStageGeometry:
			stageRsc.set_signature(formatSignature('S','G','E','O'));
			break;
		case EShaderStage::eShaderStageTessCtrl:
			stageRsc.set_signature(formatSignature('S','T','E','C'));
			break;
		case EShaderStage::eShaderStageTessEval:
			stageRsc.set_signature(formatSignature('S','T','E','V'));
			break;
		case EShaderStage::eShaderStagePixel:
			stageRsc.set_signature(formatSignature('S','P','I','X'));
			break;
		default: { cerr << "ERROR: unknown stage\n"; }
		}

		MemoryBuffer bytecode;

		//Compile with hlsl backend
		if (m_hlsl.compile(source, bytecode, entryPoint.c_str(), stage))
		{
			stageRsc.set_code_hlslSM5(stageRsc.createArray<byte>((const byte*)bytecode.pointer(), bytecode.size()));
		}
		else
		{
			cerr << "ERROR: Unable to compile HLSL stage\n";
			return false;
		}

		bytecode = MemoryBuffer();

		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderCompiler::ShaderCompiler() :
	pCompiler(new Impl())
{}

ShaderCompiler::~ShaderCompiler() { pCompiler.reset(); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int ShaderCompiler::compile(istream& source, ostream& output, const ShaderCompilerOptions& options)
{
	//Error status
	uint32 status = 0;

	tsr::ShaderBuilder shaderRsc;
	
	//TSSH
	shaderRsc.set_signature(formatSignature('T', 'S', 'S', 'H'));

	string sourceData = string(istreambuf_iterator<char>(source), istreambuf_iterator<char>());
	
	//Vertex
	if (!options.vsEntry.empty())
	{
		tsr::ShaderStageBuilder stageRsc(shaderRsc);
		bool ret = pCompiler->processStage(stageRsc, sourceData, options.vsEntry, eShaderStageVertex);
		auto ref = stageRsc.build();

		if (ret)
			shaderRsc.set_vertex(ref);
		else
			status = 1;
	}
	
	//Tesselator Control
	if (!options.tcsEntry.empty())
	{
		tsr::ShaderStageBuilder stageRsc(shaderRsc);
		bool ret = pCompiler->processStage(stageRsc, sourceData, options.tcsEntry, eShaderStageTessCtrl);
		auto ref = stageRsc.build();

		if (ret)
			shaderRsc.set_tessControl(ref);
		else
			status = 1;
	}

	//Tesselator Evaluation
	if (!options.tesEntry.empty())
	{
		tsr::ShaderStageBuilder stageRsc(shaderRsc);
		bool ret = pCompiler->processStage(stageRsc, sourceData, options.tesEntry, eShaderStageTessEval);
		auto ref = stageRsc.build();

		if (ret)
			shaderRsc.set_tessEval(ref);
		else
			status = 1;
	}
	
	//Geometry
	if (!options.gsEntry.empty())
	{
		tsr::ShaderStageBuilder stageRsc(shaderRsc);
		bool ret = pCompiler->processStage(stageRsc, sourceData, options.gsEntry, eShaderStageGeometry);
		auto ref = stageRsc.build();

		if (ret)
			shaderRsc.set_geometry(ref);
		else
			status = 1;
	}

	//Pixel
	if (!options.psEntry.empty())
	{
		tsr::ShaderStageBuilder stageRsc(shaderRsc);
		bool ret = pCompiler->processStage(stageRsc, sourceData, options.psEntry, eShaderStagePixel);
		auto ref = stageRsc.build();

		if (ret)
			shaderRsc.set_pixel(ref);
		else
			status = 1;
	}

	shaderRsc.build(output);

	if (output.fail())
	{
		return -1;
	}

	return status;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
