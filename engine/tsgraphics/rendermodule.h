/*
	Render Module header

	The rendering module is responsible for controlling the rendering pipeline and it's resources
*/

#pragma once

#include <tsconfig.h>
#include <tscore/types.h>
#include <tscore/system/memory.h>
#include <tscore/maths.h>

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	enum ERenderScreenState
	{
		eScreenDefault	  = 0,
		eScreenBorderless = 1,
		eScreenFullscreen = 2
	};
	
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
		//Screen state
		ERenderScreenState screenState = eScreenDefault;
		ERenderApiID apiEnum = ERenderApiID::eRenderApiNull;
	};

	class IRenderApi;
	
	class CRenderModule
	{
	private:

		UniquePtr<IRenderApi> m_api;

	public:

		CRenderModule(const SRenderModuleConfiguration&);
		~CRenderModule();
	
		void setScreenState(ERenderScreenState state);

		void drawBegin(const Vector& vec);
		void drawEnd();
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////