/*
	Render Module header

	The rendering module is responsible for initializing the rendering pipeline and creating graphical resources
*/

#pragma once

#include <tsconfig.h>
#include <tscore/types.h>
#include <tscore/system/memory.h>

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	struct SRenderModuleConfiguration
	{
		//(Win32) HWND of application window
		intptr targethandle = 0;
	};
	
	class CRenderModule
	{
	private:


	public:

		CRenderModule(const SRenderModuleConfiguration&);
		~CRenderModule();
	
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////