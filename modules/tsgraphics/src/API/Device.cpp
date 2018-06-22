/*
    Device interface
*/

#include <tsgraphics/Device.h>
#include "d3d11/Render.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////

RenderDevice::Ptr RenderDevice::create(RenderDeviceID id, const RenderDeviceConfig& config)
{
    return RenderDevice::Ptr(new D3D11(config));
}

void RenderDevice::destroy(RenderDevice* device)
{
    auto dptr = dynamic_cast<D3D11*>(device);
    
    if (dptr != nullptr)
    {
        delete dptr;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
