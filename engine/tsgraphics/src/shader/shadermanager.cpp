/*
	Shader manager source

	todo: implement actual lifetime management of shader resources - at the moment shader resources cannot be released
*/

#include "preprocessor.h"

#include <tsgraphics/graphicsSystem.h>
#include <tsgraphics/shadermanager.h>

#include <fstream>
#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>

using namespace std;
using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////

CShaderManager::CShaderManager(GraphicsSystem* system, uint flags, const Path& sourcepath, const Path& cachepath) :
	m_graphics(system),
	m_sourcePath(sourcepath),
	m_cachePath(cachepath),
	m_flags(flags)
{
	tsassert(m_graphics);

	abi::createShaderCompiler(&m_shaderCompiler);

	m_idcounter = 0;
}

CShaderManager::~CShaderManager()
{
	abi::destroyShaderCompiler(m_shaderCompiler);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//Compile a shader from a string
bool CShaderManager::createShaderFromString(ShaderId& id, const char* code, const char* entrypoint, EShaderStage stage)
{
	SShaderCompileConfig config;
	config.entrypoint = entrypoint;
	config.debuginfo = m_flags & eShaderManagerFlag_Debug;
	config.stage = stage;

	MemoryBuffer bytecode;
	if (!m_shaderCompiler->compile(code, config, bytecode))
		return false;

	//Set new id
	m_idcounter++;
	id = m_idcounter;

	SShaderInstance inst;
	if (ERenderStatus status = m_graphics->getApi()->createShader(inst.hShader, bytecode.pointer(), (uint32)bytecode.size(), config.stage))
	{
		tswarn("unable to load shader");
		m_graphics->getApi()->destroyShader(inst.hShader);
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
	stringstream sourcestream;

	preprocessFile(source, sourcestream);

	SShaderCompileConfig config;
	config.entrypoint = entrypoint;
	config.debuginfo = m_flags & eShaderManagerFlag_Debug;
	config.stage = stage;

	if (!m_shaderCompiler->compile(sourcestream.str().c_str(), config, bytecode))
		return false;

	//Set new id
	m_idcounter++;
	id = m_idcounter;

	SShaderInstance inst;
	if (ERenderStatus status = m_graphics->getApi()->createShader(inst.hShader, bytecode.pointer(), (uint32)bytecode.size(), config.stage))
	{
		tswarn("unable to load shader");
		m_graphics->getApi()->destroyShader(inst.hShader);
		return false;
	}

	inst.config = config;

	m_shaderInstanceMap.push_back(inst);
	m_shaderFileMap.insert(make_pair(source, id));

	return true;
}

HShader CShaderManager::getShaderHandle(ShaderId id)
{
	return m_shaderInstanceMap.at(id - 1).hShader;
}

void CShaderManager::addMacro(const string& macroname, const string& macrovalue)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////
