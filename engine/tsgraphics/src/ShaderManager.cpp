/*
	Shader manager source

	todo: implement actual lifetime management of shader resources - at the moment shader resources cannot be released
*/

#include <tsgraphics/graphicsSystem.h>
#include <tsgraphics/shadermanager.h>

#include <tsgraphics/api/RenderApi.h>

#include <tscore/filesystem/pathhelpers.h>
#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>

#include <vector>
#include <map>

using namespace std;
using namespace ts;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Shader loader
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum EShaderBackend : uint8
{
	eBackendHLSL_SM5 = 1,
	eBackendHLSL_SM5_1 = 2,
	eBackendHLSL_SM6 = 3,
	eBackendSPIRV = 4,
};

//File structures
#pragma pack(push, 1)
struct SHA256Hash
{
	uint64 a = 0;
	uint64 b = 0;
	uint64 c = 0;
	uint64 d = 0;

	operator bool() const
	{
		return a || b;
	}

	bool operator<(const SHA256Hash& rhs) const
	{
		return tie(a, b, c, d) < tie(rhs.a, rhs.b, rhs.c, rhs.d);
	}

	bool operator==(const SHA256Hash& rhs) const
	{
		return tie(a, b, c, d) == tie(rhs.a, rhs.b, rhs.c, rhs.d);
	}

	//Returns number of characters needed to represent the hash as a string excluding the null terminator
	constexpr static size_t charCount()
	{
		return sizeof(SHA256Hash) * 2;
	}
};

typedef SHA256Hash Hash;

//Shader object file header
struct SShaderObjectHeader
{
	//TSHO
	uint32 tag;

	Hash stageVertex;
	Hash stageHull;
	Hash stageDomain;
	Hash stageGeometry;
	Hash stagePixel;
};
#pragma pack(pop)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Internal implementation
struct CShaderManager::Manager
{
	GraphicsSystem* system = nullptr;

	Path shaderPath;

	vector<SShaderProgram> programs;
	map<string, ShaderId> programMap;
	map<Hash, HShader> programStageMap;

	uint flags = 0;

	//Finds shader stage in cache directory and loads it
	EShaderManagerStatus loadShaderStage(Path cacheDir, Hash stageHash, HShader& shader, EShaderStage stage);
	//Finds shader program in given file
	EShaderManagerStatus loadProgram(const string& shaderName, ShaderId& id);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	ctor/dtor
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CShaderManager::CShaderManager(GraphicsSystem* system, const Path& shaderpath, uint flags) :
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

GraphicsSystem* const CShaderManager::getSystem() const
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

string hashToStr(Hash hash)
{
	//Format 128bit hash - 32 hex chars
	char hashStr[Hash::charCount() + 1] = { 0 };
	snprintf(hashStr, sizeof(hashStr) - 1, "%llx%llx%llx%llx", hash.a, hash.b, hash.c, hash.d);

	return hashStr;
}

string idToStr(EShaderBackend id)
{
	char idStr[9] = { 0 };
	snprintf(idStr, sizeof(idStr) - 1, "%02x", id);
	return idStr;
}

EShaderManagerStatus CShaderManager::Manager::loadShaderStage(Path cacheDir, Hash stageHash, HShader& hShader, EShaderStage stage)
{
	auto it = programStageMap.find(stageHash);

	//If the stage has not already been loaded
	if (it == programStageMap.end())
	{
		//Search for shader in cache directory
		cacheDir.addDirectories(hashToStr(stageHash));
		hShader = HSHADER_NULL;

		if (!isFile(cacheDir))
		{
			return eShaderManagerStatus_StageNotFound;
		}

		//Read stage file into buffer
		ifstream stageFile(cacheDir.str(), ios::binary);

		vector<char> buffer;

		stageFile.seekg(0, ios::end);
		const streampos fileSize = stageFile.tellg();
		buffer.resize(fileSize);

		stageFile.seekg(0, ios::beg);
		stageFile.read(&buffer[0], fileSize);

		//Create shader instance
		if (ERenderStatus status = system->getApi()->createShader(hShader, &buffer[0], (uint32)buffer.size(), stage))
		{
			return eShaderManagerStatus_StageCorrupt;
		}

		//Cache shader stage
		programStageMap[stageHash] = hShader;
	}
	else
	{
		//Set stage to shader instane found in cache
		hShader = it->second;

		if (hShader == HSHADER_NULL)
		{
			return eShaderManagerStatus_StageNotFound;
		}
	}

	return eShaderManagerStatus_Ok;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Manager methods

EShaderManagerStatus CShaderManager::Manager::loadProgram(const string& shaderName, ShaderId& id)
{
	Path shaderFile = shaderPath;
	shaderFile.addDirectories(shaderName + ".tsh");

	auto it = programMap.find(shaderName);

	if (it == programMap.end())
	{
		if (!isFile(shaderFile))
		{
			return eShaderManagerStatus_ProgramNotFound;
		}

		ifstream file(shaderFile.str(), ios::binary);

		//Read header
		SShaderObjectHeader header;
		file.read((char*)&header, sizeof(SShaderObjectHeader));

		if (file.fail())
		{
			return eShaderManagerStatus_Fail;
		}

		//Validate header
		if (header.tag != 'OHST')
		{
			return eShaderManagerStatus_StageCorrupt;
		}

		//Determine location of shader programs based in render api selected

		SGraphicsSystemConfig config;
		system->getConfiguration(config);

		EShaderBackend backendId;
		switch (config.apiid)
		{
		case eGraphicsAPI_D3D11:
			backendId = EShaderBackend::eBackendHLSL_SM5;
			break;
		default:
			backendId = (EShaderBackend)0;
		}

		Path cacheDir = shaderPath;
		cacheDir.addDirectories("cache");
		cacheDir.addDirectories(idToStr(backendId));

		if (!isDirectory(cacheDir))
		{
			tswarn("a shader cache directory could not be found for this Renderer configuration (%)", config.apiid);
			return eShaderManagerStatus_StageNotFound;
		}

		//Load each stage if their hashes are not null
		SShaderProgram program;

		if (header.stageVertex)
		{
			if (auto err = loadShaderStage(cacheDir, header.stageVertex, program.hVertex, EShaderStage::eShaderStageVertex))
				return err;
		}

		if (header.stageHull)
		{
			if (auto err = loadShaderStage(cacheDir, header.stageHull, program.hHull, EShaderStage::eShaderStageHull))
				return err;
		}

		if (header.stageDomain)
		{
			if (auto err = loadShaderStage(cacheDir, header.stageDomain, program.hDomain, EShaderStage::eShaderStageDomain))
				return err;
		}

		if (header.stageGeometry)
		{
			if (auto err = loadShaderStage(cacheDir, header.stageGeometry, program.hGeometry, EShaderStage::eShaderStageGeometry))
				return err;
		}

		if (header.stagePixel)
		{
			if (auto err = loadShaderStage(cacheDir, header.stagePixel, program.hPixel, EShaderStage::eShaderStagePixel))
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
