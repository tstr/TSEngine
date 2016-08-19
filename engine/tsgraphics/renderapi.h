/*
	Render API

	The render api acts as a layer between the rendering module and the low level graphics implementation (D3D11 in this case)
*/

#pragma once

#include <tscore/types.h>
#include <tscore/maths.h>
#include <tscore/strings.h>

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	enum ERenderApiFlags : uint16
	{
		eFlagNull =  0,
		eFlagDebug = 1
	};

	enum EWindowMode
	{
		eWindowDefault = 0,
		eWindowBorderless = 1,
		eWindowFullscreen = 2
	};

	struct SRenderAdapterDesc
	{
		StaticString<128> adapterName;
		uint64 gpuVideoMemory;		//GPU accessible video memory capacity
		uint64 gpuSystemMemory;		//GPU accessible system memory capacity
		uint64 sharedSystemMemory;	//GPU/CPU accessible system memory capacity
	};

	struct SRenderApiConfiguration
	{
		intptr windowHandle = 0;
		uint32 adapterIndex = 0;
		uint32 resolutionWidth = 0;
		uint32 resolutionHeight = 0;
		EWindowMode windowMode = eWindowDefault;
		uint16 flags = 0;
	};


	class IRenderApi
	{
	public:

		virtual void setWindowMode(EWindowMode mode) = 0;

		virtual void drawBegin(const Vector& vec) = 0;
		virtual void drawEnd() = 0;

		virtual ~IRenderApi() {}
	};


	class IRenderContext
	{
	public:


	};


	class IRenderAdapterFactory
	{
	public:

		virtual uint32 getAdapterCount() const = 0;
		virtual bool enumAdapter(uint32 idx, SRenderAdapterDesc& desc) const = 0;
	};

}

/////////////////////////////////////////////////////////////////////////////////////////////////