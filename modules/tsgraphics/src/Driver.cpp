/*
    Device interface
*/

#include <tsgraphics/Driver.h>
#include <tsdx11.h> //d3d11 driver

using namespace ts;

////////////////////////////////////////////////////////////////////////////////////////////////////////////

RenderDevice::Ptr RenderDevice::create(RenderDriverID id, const RenderDeviceConfig& config)
{
	switch (id)
	{
	case RenderDriverID::DX11:
		return RenderDevice::Ptr(createDX11device(config));
	default:
		return RenderDevice::Ptr();
	}
}

void RenderDevice::destroy(RenderDevice* device)
{
	//Hardcode for now to dx11
	destroyDX11device(device);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
