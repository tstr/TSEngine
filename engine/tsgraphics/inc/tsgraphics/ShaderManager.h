/*
	Shader manager header
*/

#pragma once

#include <tsgraphics/abi.h>
#include <tsgraphics/api/renderapi.h>
#include <tsgraphics/api/rendercommon.h>

#include <tscore/filesystem/path.h>
#include <tscore/strings.h>

#include <vector>
#include <unordered_map>
#include <mutex>

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	class GraphicsSystem;
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
		GraphicsSystem* m_graphics = nullptr;
		
		Path m_sourcePath;
		Path m_cachePath;
		uint m_flags = 0;

		uint64 m_idcounter = 0;

		struct SShaderInstance
		{
			HShader hShader;
			SShaderCompileConfig config;
		};

		std::vector<SShaderInstance> m_shaderInstanceMap;		 //Maps shader id's to shader resource proxies
		std::unordered_multimap<Path, ShaderId> m_shaderFileMap; //Maps shader file paths to shader id's
		std::unordered_map<std::string, std::string> m_shaderMacroMap;
		
	public:
		
		TSGRAPHICS_API CShaderManager(GraphicsSystem* module, uint flags, const Path& sourcepath = "", const Path& cachepath = "");
		~CShaderManager() { clear(); }

		TSGRAPHICS_API void clear();

		GraphicsSystem* const getModule() const { return m_graphics; }
		
		uint getFlags() const { return m_flags; }
		void setFlags(uint f) { m_flags = f; }

		TSGRAPHICS_API void addMacro(const std::string& macroname, const std::string& macrovalue);

		void setSourcepath(const Path& sourcepath) { m_sourcePath = sourcepath; }
		void setCachepath(const Path& cachepath) { m_cachePath = cachepath; }

		TSGRAPHICS_API HShader getShaderHandle(ShaderId id);

		TSGRAPHICS_API bool createShaderFromString(ShaderId& id, const char* code, const char* entrypoint, EShaderStage stage);
		TSGRAPHICS_API bool createShaderFromFile(ShaderId& id, const Path& codefile, const char* entrypoint, EShaderStage stage);
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////