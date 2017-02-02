/*
	Shader manager source

	todo: implement actual lifetime management of shader resources - at the moment shader resources cannot be released
*/

#include <tsgraphics/graphicsSystem.h>
#include <tsgraphics/shadermanager.h>

#include <tscore/filesystem/pathhelpers.h>
#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>

#include <vector>
#include <map>

using namespace std;
using namespace ts;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Internal implementation
struct CShaderManager::Manager
{
	GraphicsSystem* system = nullptr;

	Path shaderPath;

	vector<SShaderProgram> programs;
	map<string, ShaderId> programMap;

	uint flags = 0;

	EShaderManagerStatus loadFromFile(const Path& shaderFile, SShaderProgram& program);
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

		delete pManage;
		pManage = nullptr;
	}
}

CShaderManager::CShaderManager(CShaderManager&& rhs)
{
	swap(pManage, rhs.pManage);
}

CShaderManager& CShaderManager::operator=(CShaderManager&& rhs)
{
	swap(pManage, rhs.pManage);
	return *this;
}

void CShaderManager::clear()
{
	if (pManage)
	{
		auto gfx = pManage->system;

		for (SShaderProgram& p : pManage->programs)
		{
			if (p.shVertex)   gfx->getApi()->destroyShader(p.shVertex);
			if (p.shHull)     gfx->getApi()->destroyShader(p.shHull);
			if (p.shDomain)   gfx->getApi()->destroyShader(p.shDomain);
			if (p.shGeometry) gfx->getApi()->destroyShader(p.shGeometry);
			if (p.shPixel)    gfx->getApi()->destroyShader(p.shPixel);
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
};

struct SShaderObjectHeader
{
	//TSHO
	uint32 tag;

	SHA256Hash stageVertex;
	SHA256Hash stageHull;
	SHA256Hash stageDomain;
	SHA256Hash stageGeometry;
	SHA256Hash stagePixel;
};
#pragma pack(pop)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

string hashToStr(SHA256Hash hash)
{
	//Format 128bit hash - 32 hex chars
	char hashStr[sizeof(SHA256Hash) * 2 + 1] = { 0 };
	snprintf(hashStr, sizeof(hashStr) - 1, "%llx%llx%llx%llx", hash.a, hash.b, hash.c, hash.d);

	return hashStr;
}

string idToStr(EShaderBackend id)
{
	char idStr[9] = { 0 };
	snprintf(idStr, sizeof(idStr) - 1, "%02x", id);
	return idStr;
}

EShaderManagerStatus loadShaderStage(IRender* api, Path cacheDir, SHA256Hash stageHash, HShader& shader, EShaderStage stage)
{
	cacheDir.addDirectories(hashToStr(stageHash));
	shader = HSHADER_NULL;

	if (!isFile(cacheDir))
	{
		return eShaderManagerStatus_StageNotFound;
	}

	ifstream stageFile(cacheDir.str(), ios::binary);

	vector<char> buffer;

	stageFile.seekg(0, ios::end);
	streampos fileSize = stageFile.tellg();
	buffer.resize(fileSize);

	stageFile.seekg(0, ios::beg);
	stageFile.read(&buffer[0], fileSize);
	
	if (ERenderStatus status = api->createShader(shader, &buffer[0], (uint32)buffer.size(), stage))
	{
		return eShaderManagerStatus_CorruptShader;
	}

	return eShaderManagerStatus_Ok;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Manager methods

EShaderManagerStatus CShaderManager::Manager::loadFromFile(const Path& fileName, SShaderProgram& program)
{
	Path shaderFile = shaderPath;
	shaderFile.addDirectories(fileName);

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
		return eShaderManagerStatus_CorruptShader;
	}

	//Determine location of shader programs based in render api selected

	SGraphicsSystemConfig config;
	system->getConfiguration(config);

	EShaderBackend backendId;
	switch (config.apiEnum)
	{
	case eRenderApiD3D11:
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
		tswarn("a shader cache directory could not be found for this Renderer configuration (%)", config.apiEnum);
		return eShaderManagerStatus_StageNotFound;
	}

	//Load each stage if their hashes are not null

	if (header.stageVertex)
	{
		if (auto err = loadShaderStage(system->getApi(), cacheDir, header.stageVertex, program.shVertex, EShaderStage::eShaderStageVertex))
			return err;
	}

	if (header.stageHull)
	{
		if (auto err = loadShaderStage(system->getApi(), cacheDir, header.stageHull, program.shHull, EShaderStage::eShaderStageHull))
			return err;
	}

	if (header.stageDomain)
	{
		if (auto err = loadShaderStage(system->getApi(), cacheDir, header.stageDomain, program.shDomain, EShaderStage::eShaderStageDomain))
			return err;
	}

	if (header.stageGeometry)
	{
		if (auto err = loadShaderStage(system->getApi(), cacheDir, header.stageGeometry, program.shGeometry, EShaderStage::eShaderStageGeometry))
			return err;
	}

	if (header.stagePixel)
	{
		if (auto err = loadShaderStage(system->getApi(), cacheDir, header.stagePixel, program.shPixel, EShaderStage::eShaderStagePixel))
			return err;
	}

	string name = fileName.str();
	name.erase(name.find_last_of('.'));

	programs.push_back(program);

	return eShaderManagerStatus_Ok;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EShaderManagerStatus CShaderManager::load(const char* shaderName, SShaderProgram& program)
{
	if (!pManage)
	{
		return eShaderManagerStatus_NullManager;
	}
	
	return pManage->loadFromFile((string)shaderName + ".tsh", program);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
