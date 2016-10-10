/*
	Shader manager header
*/

#pragma once

#include <tsgraphics/rendercommon.h>
#include <tsgraphics/renderapi.h>

#include <tscore/filesystem/path.h>
#include <tscore/strings.h>

#include <vector>
#include <unordered_map>
#include <mutex>

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	class CRenderModule;
	class CShaderManager;

	enum EShaderManagerFlags
	{
		eShaderManagerFlag_Null		 = (0),
		eShaderManagerFlag_Debug 	 = (1 << 0),
		eShaderManagerFlag_Recompile = (1 << 1),
		eShaderManagerFlag_FileWatch = (1 << 2),
	};

	typedef uint64 ShaderId;

	class CShaderManager
	{
	private:

		IShaderCompiler* m_shaderCompiler = nullptr;
		CRenderModule* m_renderModule = nullptr;
		
		Path m_sourcePath;
		Path m_cachePath;
		uint m_flags = 0;

		uint64 m_idcounter = 0;

		struct SShaderInstance
		{
			ResourceProxy proxy;
			SShaderCompileConfig config;
		};

		std::vector<SShaderInstance> m_shaderInstanceMap;		 //Maps shader id's to shader resource proxies
		std::unordered_multimap<Path, ShaderId> m_shaderFileMap; //Maps shader file paths to shader id's
		std::unordered_map<std::string, std::string> m_shaderMacroMap;
		
	public:
		
		CShaderManager(CRenderModule* module, uint flags, const Path& sourcepath = "", const Path& cachepath = "");
		~CShaderManager();

		CRenderModule* const getModule() const { return m_renderModule; }
		
		uint getFlags() const { return m_flags; }
		void setFlags(uint f) { m_flags = f; }

		void setSourcepath(const Path& sourcepath) { m_sourcePath = sourcepath; }
		void setCachepath(const Path& cachepath) { m_cachePath = cachepath; }

		ResourceProxy& getShaderProxy(ShaderId id);

		bool createShaderFromString(ShaderId& id, const char* code, const char* entrypoint, EShaderStage stage);
		bool createShaderFromFile(ShaderId& id, const Path& codefile, const char* entrypoint, EShaderStage stage);
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////