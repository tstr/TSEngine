/*
	DirectX 11 Rendering API main header
*/

#pragma once

#include <dxgi.h>
#include <d3d11.h>
#include <d3d11_2.h>

#include <wrl/client.h> //ComPtr
#include <comdef.h>		//_com_error

#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>

#define IID_OF(i) __uuidof(i)

namespace ts
{
	namespace dx11
	{
		using Microsoft::WRL::ComPtr;

		inline void setObjectDebugName(ID3D11DeviceChild* child, const char* name)
		{
			//using namespace DirectX;

#ifdef _DEBUG

			if (child != nullptr && name != nullptr)
				D3D_SET_OBJECT_NAME_A(child, name);

#endif

		}
	}
}