/*
	Shader manager source

	todo: implement actual lifetime management of shader resources - at the moment shader resources cannot be released
*/

#include <tsgraphics/shadermanager.h>
#include <tsgraphics/api/RenderApi.h>

#include <tscore/pathutil.h>
#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>

#include <vector>
#include <map>

#include "tsgraphics/schemas/Shader.rcs.h"

using namespace std;
using namespace ts;
using namespace tsr;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Functions
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline uint32 formatSignature(char a, char b, char c, char d)
{
	return (d << 24) | (c << 16) | (b << 8) | (a << 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Internal implementation
struct CShaderManager::Manager
{
	GraphicsCore* system = nullptr;

	Path shaderPath;

	vector<SShaderProgram> programs;
	map<string, ShaderId> programMap;

	uint flags = 0;

	//Load compiled shader stage from resource
	EShaderManagerStatus loadProgramStage(HShader& shader, EShaderStage stageEnum, const tsr::ShaderStage& stage);
	//Finds shader program in given file
	EShaderManagerStatus loadProgram(const string& shaderName, ShaderId& id);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	ctor/dtor
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CShaderManager::CShaderManager(GraphicsCore* system, const Path& shaderpath, uint flags) :
	pManage(new Manager())
{
	tsassert(system);

	pManage->system = system;
	pManage->shaderPath = shaderpath;
	pManage->flags = flags;
}

CShaderManager::~CShaderManager()
{
	if (pManage)
	{
		clear();
		pManage.reset();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Clears shader cache and deallocates
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CShaderManager::clear()
{
	if (pManage)
	{
		auto gfx = pManage->system;

		for (SShaderProgram& p : pManage->programs)
		{
			if (p.hVertex)   gfx->getApi()->destroyShader(p.hVertex);
			if (p.hHull)     gfx->getApi()->destroyShader(p.hHull);
			if (p.hDomain)   gfx->getApi()->destroyShader(p.hDomain);
			if (p.hGeometry) gfx->getApi()->destroyShader(p.hGeometry);
			if (p.hPixel)    gfx->getApi()->destroyShader(p.hPixel);
		}

		pManage->programs.clear();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Properties
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GraphicsCore* const CShaderManager::getSystem() const
{
	return pManage->system;
}

uint CShaderManager::getFlags() const
{
	return pManage->flags;
}

void CShaderManager::setFlags(uint f)
{
	pManage->flags = f;
}

void CShaderManager::setLoadPath(const Path& shaderpath)
{
	pManage->shaderPath = shaderpath;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EShaderManagerStatus CShaderManager::Manager::loadProgramStage(HShader& hShader, EShaderStage stageEnum, const tsr::ShaderStage& stage)
{
	uint32 signature = 0;

	switch (stageEnum)
	{
	case EShaderStage::eShaderStageVertex:
		signature = formatSignature('S', 'V', 'T', 'X');
		break;
	case EShaderStage::eShaderStageGeometry:
		signature = formatSignature('S', 'G', 'E', 'O');
		break;
	case EShaderStage::eShaderStageTessCtrl:
		signature = formatSignature('S', 'T', 'E', 'C');
		break;
	case EShaderStage::eShaderStageTessEval:
		signature = formatSignature('S', 'T', 'E', 'V');
		break;
	case EShaderStage::eShaderStagePixel:
		signature = formatSignature('S', 'P', 'I', 'X');
		break;
	}

	//Determine which compiled stages to load
	const EGraphicsAPIID apiid = system->getApiID();

	//Verify stage signature
	if (signature != stage.signature())
	{
		return eShaderManagerStatus_StageCorrupt;
	}

	switch (apiid)
	{
	case eGraphicsAPI_D3D11:

		//If stage is present
		if (stage.has_code_hlslSM5())
		{
			//Create stage
			ERenderStatus status = system->getApi()->createShader(
				hShader,
				stage.code_hlslSM5().data(),
				(uint32)stage.code_hlslSM5().length(),
				stageEnum
			);

			if (status)
			{
				return eShaderManagerStatus_StageCorrupt;
			}
		}
		else
		{
			//Exit failure
			return eShaderManagerStatus_StageCorrupt;
		}

		break;

	default:
		return eShaderManagerStatus_StageNotFound;
	}

	return eShaderManagerStatus_Ok;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Manager methods

EShaderManagerStatus CShaderManager::Manager::loadProgram(const string& shaderName, ShaderId& id)
{
	Path shaderFile = shaderPath;
	shaderFile.addDirectories(shaderName + ".shader");

	auto it = programMap.find(shaderName);

	if (it == programMap.end())
	{
		if (!isFile(shaderFile))
		{
			return eShaderManagerStatus_ProgramNotFound;
		}

		rc::ResourceLoader loader(ifstream(shaderFile.str(), ios::binary));

		if (loader.fail())
		{
			return eShaderManagerStatus_Fail;
		}

		tsr::Shader& shaderRsc = loader.deserialize<tsr::Shader>();

		//Verify signature
		if (shaderRsc.signature() != formatSignature('T', 'S', 'S', 'H'))
		{
			return eShaderManagerStatus_StageCorrupt;
		}

		//Load each stage if they exist
		SShaderProgram program;

		if (shaderRsc.has_vertex())
		{
			if (auto err = loadProgramStage(program.hVertex, EShaderStage::eShaderStageVertex, shaderRsc.vertex()))
				return err;
		}

		if (shaderRsc.has_tessControl())
		{
			if (auto err = loadProgramStage(program.hHull, EShaderStage::eShaderStageTessCtrl, shaderRsc.tessControl()))
				return err;
		}

		if (shaderRsc.has_tessEval())
		{
			if (auto err = loadProgramStage(program.hDomain, EShaderStage::eShaderStageTessEval, shaderRsc.tessEval()))
				return err;
		}

		if (shaderRsc.has_geometry())
		{
			if (auto err = loadProgramStage(program.hGeometry, EShaderStage::eShaderStageGeometry, shaderRsc.geometry()))
				return err;
		}

		if (shaderRsc.has_pixel())
		{
			if (auto err = loadProgramStage(program.hPixel, EShaderStage::eShaderStagePixel, shaderRsc.pixel()))
				return err;
		}

		programs.push_back(program);
		id = (uint32)programs.size(); //ShaderId is equal to the index of the shader instance + 1 ie. the new size of the program cache
		programMap.insert(make_pair(shaderName, id));
	}
	else
	{
		id = it->second;
	}

	return eShaderManagerStatus_Ok;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EShaderManagerStatus CShaderManager::load(const char* shaderName, ShaderId& id)
{
	if (!pManage)
	{
		return eShaderManagerStatus_NullManager;
	}
	
	return pManage->loadProgram(shaderName, id);
}

EShaderManagerStatus CShaderManager::getProgram(ShaderId id, SShaderProgram& prog)
{
	if (!pManage)
	{
		return eShaderManagerStatus_NullManager;
	}

	uint32 idx = id - 1;

	if (idx >= (uint32)pManage->programs.size())
	{
		return eShaderManagerStatus_ProgramNotFound;
	}

	prog = pManage->programs.at(idx);

	return eShaderManagerStatus_Ok;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
