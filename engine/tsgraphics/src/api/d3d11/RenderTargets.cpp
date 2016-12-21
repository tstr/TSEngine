/*
	Render API

	D3D11Render target handling methods
*/

#include "render.h"

using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ERenderStatus D3D11Render::createTarget(
	HTarget& target,
	const HTexture* renderTexture,
	const uint32* renderTextureIndices,
	uint32 renderTextureCount,
	HTexture deptTextureProxy,
	uint32 deptTextureProxyIndex
)
{

}

void D3D11Render::destroyTarget(HTarget target)
{
	if (auto u = reinterpret_cast<IUnknown*>(target))
	{
		u->Release();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
