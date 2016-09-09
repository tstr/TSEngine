/*
	Shader manager source
*/

#include "shadermanager.h"
#include "rendermodule.h"

#include <fstream>
#include <tscore/debug/assert.h>

#include "API/DX11/DX11render.h"

using namespace std;
using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////

CShader::CShader(
	CShaderManager* manager,
	const MemoryBuffer& bytecode,
	EShaderStage stage
) :
	m_manager(manager)
{
	if (ERenderStatus status = m_manager->getModule()->getApi()->createShader(m_shader, bytecode.pointer(), (uint32)bytecode.size(), stage))
	{
		tswarn("unable to load shader");
		m_shader = ResourceProxy();
	}
}

CShader::~CShader()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////

CShaderManager::CShaderManager(CRenderModule* module, const Path& rootpath) :
	m_renderModule(module),
	m_rootpath(rootpath)
{
	tsassert(module);

	m_shaderCompiler = new dx11::DX11ShaderCompiler;
}

CShaderManager::~CShaderManager()
{
	delete m_shaderCompiler;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

bool CShaderManager::compileAndLoadShader(CShader& shader, const char* code, const SShaderCompileConfig& config)
{
	MemoryBuffer bytecode;
	if (!m_shaderCompiler->compile(code, config, bytecode))
		return false;
	shader = CShader(this, bytecode, config.stage);

	return true;
}

bool CShaderManager::compileAndLoadShaderFile(CShader& shader, const Path& codefile, const SShaderCompileConfig& config)
{
	MemoryBuffer bytecode;

	Path root(m_rootpath);
	root.addDirectories(codefile);
	ifstream filestream(root.str());

	string buf((istreambuf_iterator<char>(filestream)), istreambuf_iterator<char>());

	if (!m_shaderCompiler->compile(buf.c_str(), config, bytecode))
		return false;
	shader = CShader(this, bytecode, config.stage);

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////