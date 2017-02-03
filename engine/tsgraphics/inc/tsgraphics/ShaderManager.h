/*
	Shader manager header
*/

#pragma once

#include <tsgraphics/abi.h>
#include <tsgraphics/api/renderapi.h>
#include <tsgraphics/api/rendercommon.h>

#include <tscore/ptr.h>
#include <tscore/filesystem/path.h>
#include <tscore/strings.h>

#include <vector>

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	class GraphicsSystem;
	class CShaderManager;

	enum EShaderManagerFlags
	{
		eShaderManagerFlag_Null		 = (0 << 0),
		eShaderManagerFlag_Debug 	 = (1 << 0),
		eShaderManagerFlag_NoPreload = (1 << 1),
		eShaderManagerFlag_FileWatch = (1 << 2),
	};

	enum EShaderManagerStatus
	{
		eShaderManagerStatus_Ok				 = 0,
		eShaderManagerStatus_Fail			 = 1,
		eShaderManagerStatus_NullManager	 = 2,
		eShaderManagerStatus_ProgramNotFound = 3,
		eShaderManagerStatus_StageNotFound   = 4,
		eShaderManagerStatus_StageCorrupt	 = 5
	};

	struct SShaderProgram
	{
		HShader shVertex = HSHADER_NULL;
		HShader shDomain = HSHADER_NULL;
		HShader shHull = HSHADER_NULL;
		HShader shGeometry = HSHADER_NULL;
		HShader shPixel = HSHADER_NULL;
	};

	typedef uint64 ShaderId;
	
	class CShaderManager
	{
	private:

		struct Manager;
		OpaquePtr<Manager> pManage;

	public:

		OPAQUE_PTR(CShaderManager, pManage)
		
		CShaderManager() {}

		TSGRAPHICS_API CShaderManager(GraphicsSystem* system, const Path& shaderPath, uint flags);
		TSGRAPHICS_API ~CShaderManager();
		
		TSGRAPHICS_API void clear();
		
		TSGRAPHICS_API GraphicsSystem* const getSystem() const;
		TSGRAPHICS_API void setLoadPath(const Path& shaderpath);

		TSGRAPHICS_API uint getFlags() const;
		TSGRAPHICS_API void setFlags(uint f);


		TSGRAPHICS_API EShaderManagerStatus load(const char* shaderName, SShaderProgram& program);
		EShaderManagerStatus load(const std::string& shaderName, SShaderProgram& program) { return this->load(shaderName.c_str(), program); }
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////