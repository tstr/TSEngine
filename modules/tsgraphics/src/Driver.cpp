/*
    Device interface
*/

#include <tsgraphics/Driver.h>
#include "api/d3d11/Render.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////

RenderDevice::Ptr RenderDevice::create(RenderDriverID id, const RenderDeviceConfig& config)
{
	switch (id)
	{
	case RenderDriverID::DX11:
		return RenderDevice::Ptr(new Dx11(config));
	default:
		return RenderDevice::Ptr();
	}
}

void RenderDevice::destroy(RenderDevice* device)
{
    auto dptr = dynamic_cast<Dx11*>(device);
    
    if (dptr != nullptr)
    {
        delete dptr;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
