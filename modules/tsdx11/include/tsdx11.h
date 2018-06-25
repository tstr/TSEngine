/*
    Public interface for DirectX 11 rendering backend
*/

#include <tsgraphics/Driver.h>

extern "C"
{
    ts::RenderDevice* createDX11device(const ts::RenderDeviceConfig& config);
    
    void destroyDX11device(ts::RenderDevice* device);
}
