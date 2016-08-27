/*
	Render Module header

	The rendering module is responsible for controlling the rendering pipeline and it's resources
*/

#pragma once

#include <tsconfig.h>
#include <tscore/system/memory.h>
#include <tscore/filesystem/path.h>
#include <tscore/maths.h>

#include "rendercommon.h"
#include "renderapi.h"

#include "shadermanager.h"
#include "texturemanager.h"

#include "indexbuffer.h"
#include "vertexbuffer.h"
#include "uniformbuffer.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	enum ERenderApiID
	{
		eRenderApiNull	= 0,
		eRenderApiD3D11 = 1
	};

	struct SRenderModuleConfiguration
	{
		//Handle to screen (application window)
		intptr windowHandle = 0;
		//Screen dimensions
		uint32 width = 0;
		uint32 height = 0;
		EWindowMode windowMode = eWindowDefault;
		ERenderApiID apiEnum = ERenderApiID::eRenderApiNull;
		//Root asset loading path for textures/shaders/models
		Path rootpath;
	};

	class IRenderApi;
	
	class CRenderModule
	{
	private:

		SRenderModuleConfiguration m_config;
		UniquePtr<IRenderApi> m_api;

		CTextureManager m_textureManager;
		CShaderManager m_shaderManager;

		bool loadApi(ERenderApiID id);

	public:

		CRenderModule(const SRenderModuleConfiguration&);
		~CRenderModule();

		CRenderModule(const CRenderModule&) = delete;
		CRenderModule& operator=(const CRenderModule&) = delete;

		CTextureManager& getTextureManager() { return m_textureManager; }
		CShaderManager& getShaderManager() { return m_shaderManager; }
		IRenderApi* const getApi() const { return m_api.get(); }

		void setWindowMode(EWindowMode mode);
		void getConfiguration(SRenderModuleConfiguration& cfg) { cfg = m_config; }

		void drawBegin(const Vector& vec);
		void drawEnd();
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////