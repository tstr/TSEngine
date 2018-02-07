/*
	Render API
	
	D3D11Texture class
*/

#pragma once

#include "base.h"
#include "handle.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	class D3D11Texture : public Handle<D3D11Texture, HTexture>
	{
	private:

		struct STextureView
		{
			uint32 arrayIndex = 0;
			uint32 arrayCount = 0;
			ETextureResourceType type;

			ComPtr<ID3D11ShaderResourceView> view;

			bool operator==(const STextureView& rhs)
			{
				bool r = true;
				r = r && (arrayIndex == rhs.arrayIndex);
				r = r && (arrayCount == rhs.arrayCount);
				r = r && (type == rhs.type);
				return r;
			}
		};
		
		ComPtr<ID3D11Resource> m_tex;
		std::vector<STextureView> m_texViewCache;

	public:
		
		D3D11Texture() {}
		D3D11Texture(ID3D11Resource* tex) : m_tex(tex) {}
		~D3D11Texture() {}
		
		ComPtr<ID3D11Resource> getResource() const
		{
			return m_tex.Get();
		}
		
		ComPtr<ID3D11ShaderResourceView> getView(uint32 arrayIndex, uint32 arrayCount, ETextureResourceType type);
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////
