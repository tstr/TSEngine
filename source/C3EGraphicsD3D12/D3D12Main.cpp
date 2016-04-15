/*
	DirectX 12 implementation
*/

#include "pch.h"

#define C3E_GFX_RENDER_API EXPORT_ATTRIB
#include <C3E\gfx\abi\graphicsabi.h>

//#include <d3d12.h>

using namespace C3E;
using namespace C3E::ABI;
using namespace std;

//////////////////////////////////////////////////////////////////////////////////////

class D3D12Render : public IRenderApi
{
public:

	D3D12Render(const Graphics::Configuration& cfg) { }
	~D3D12Render() {}
};

//////////////////////////////////////////////////////////////////////////////////////