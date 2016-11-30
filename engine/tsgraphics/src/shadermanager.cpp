/*
	Shader manager source

	todo: implement actual lifetime management of shader resources - at the moment shader resources cannot be released
*/

#include "shadermanager.h"

#include <fstream>
#include <tscore/debug/assert.h>

#include "API/DX11/DX11render.h"

using namespace std;
using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////

CShaderManager::CShaderManager(CRenderModule* module, uint flags, const Path& sourcepath, const Path& cachepath) :
	m_renderModule(module),
	m_sourcePath(sourcepath),
	m_cachePath(cachepath),
	m_flags(flags)
{
	tsassert(module);

	m_shaderCompiler = new dx11::DX11ShaderCompiler;

	m_idcounter = 0;
}

CShaderManager::~CShaderManager()
{
	delete m_shaderCompiler;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//Compile a shader from a string
bool CShaderManager::createShaderFromString(ShaderId& id, const char* code, const char* entrypoint, EShaderStage stage)
{
	SShaderCompileConfig config;
	config.entrypoint.set(entrypoint);
	config.debuginfo = m_flags & eShaderManagerFlag_Debug;
	config.stage = stage;

	MemoryBuffer bytecode;
	if (!m_shaderCompiler->compile(code, config, bytecode))
		return false;

	//Set new id
	m_idcounter++;
	id = m_idcounter;

	SShaderInstance inst;
	if (ERenderStatus status = m_renderModule->getApi()->createShader(inst.proxy, bytecode.pointer(), (uint32)bytecode.size(), config.stage))
	{
		tswarn("unable to load shader");
		inst.proxy = ResourceProxy();
		return false;
	}

	m_shaderInstanceMap.push_back(inst);

	return true;
}

//Compile a shader from a file
bool CShaderManager::createShaderFromFile(ShaderId& id, const Path& codefile, const char* entrypoint, EShaderStage stage)
{
	MemoryBuffer bytecode;

	Path source(m_sourcePath);
	source.addDirectories((string)codefile.str() + ".fx");
	ifstream filestream(source.str());

	SShaderCompileConfig config;
	config.entrypoint.set(entrypoint);
	config.sourcename.set(source.str());
	config.debuginfo = m_flags & eShaderManagerFlag_Debug;
	config.stage = stage;

	string buf((istreambuf_iterator<char>(filestream)), istreambuf_iterator<char>());

	if (!m_shaderCompiler->compile(buf.c_str(), config, bytecode))
		return false;

	//Set new id
	m_idcounter++;
	id = m_idcounter;

	SShaderInstance inst;
	if (ERenderStatus status = m_renderModule->getApi()->createShader(inst.proxy, bytecode.pointer(), (uint32)bytecode.size(), config.stage))
	{
		tswarn("unable to load shader");
		inst.proxy = ResourceProxy();
		return false;
	}

	inst.config = config;

	m_shaderInstanceMap.push_back(inst);
	m_shaderFileMap.insert(make_pair(source, id));

	return true;
}

ResourceProxy& CShaderManager::getShaderProxy(ShaderId id)
{
	return m_shaderInstanceMap.at(id - 1).proxy;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
