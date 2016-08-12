/*
	Render API

	The render api acts as a layer between the rendering module and the low level graphics implementation (D3D11)
*/

#pragma once

#include <tscore/types.h>
#include <tscore/maths.h>

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	struct SRenderApiConfiguration
	{
		intptr windowHandle = 0;
		uint32 adapterIndex = 0;
		uint32 resolutionWidth = 0;
		uint32 resolutionHeight = 0;
		bool windowFullscreen = false;
	};
	
	class IRenderApi
	{
	public:

		virtual void drawBegin(const Vector& vec) = 0;
		virtual void drawEnd() = 0;

		virtual ~IRenderApi() {}
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////