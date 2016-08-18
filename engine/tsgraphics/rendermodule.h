/*
	Render Module header

	The rendering module is responsible for controlling the rendering pipeline and it's resources
*/

#pragma once

#include <tsconfig.h>
#include <tscore/types.h>
#include <tscore/system/memory.h>
#include <tscore/maths.h>

#include "renderapi.h"

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
	};

	class IRenderApi;
	
	class CRenderModule
	{
	private:

		SRenderModuleConfiguration m_config;
		UniquePtr<IRenderApi> m_api;

		bool loadApi(ERenderApiID id);

	public:

		CRenderModule(const SRenderModuleConfiguration&);
		~CRenderModule();
		
		void setWindowMode(EWindowMode mode);

		void drawBegin(const Vector& vec);
		void drawEnd();
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////