/*
	Render API
	
	Resource object

	Can be an image or buffer
*/

#pragma once

#include <unordered_map>

#include "Base.h"
#include "Handle.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	class DxResource : public Handle<DxResource, ResourceHandle>
	{
	private:

		ComPtr<ID3D11Resource> m_rsc;

		struct SRVKey
		{
			uint32 arrayIndex = 0;
			uint32 arrayCount = 0;
			ImageType type;

			//key comp
			bool operator==(const SRVKey& other) const
			{
				return other.arrayCount == arrayCount && other.arrayIndex == arrayIndex && other.type == type;
			}

			//key hash
			size_t operator()(const SRVKey& srv) const
			{
				return (((size_t)srv.type * 8) + srv.arrayCount) * 310348631 + srv.arrayIndex;
			}
		};

		template<typename Key, typename Interface, typename Hash = std::hash<Key>>
		using Cache = std::unordered_map<Key, ComPtr<Interface>, Hash>;

		/*
			View caches
		*/
		Cache<SRVKey, ID3D11ShaderResourceView, SRVKey> m_srvCache;
		Cache<uint32, ID3D11RenderTargetView> m_rtvCache;
		Cache<uint32, ID3D11DepthStencilView> m_dsvCache;

	public:

		DxResource(ComPtr<ID3D11Resource> rsc) : m_rsc(rsc) {}
		~DxResource() { reset(); }
		
		ID3D11Resource* asResource() const { return m_rsc.Get(); }
		ID3D11Buffer* asBuffer() const { return isBuffer() ? static_cast<ID3D11Buffer*>(m_rsc.Get()) : nullptr; }
		bool isImage() const { return !isBuffer(); }
		bool isBuffer() const { return getType() == D3D11_RESOURCE_DIMENSION_BUFFER; }

		D3D11_RESOURCE_DIMENSION getType() const
		{
			if (!m_rsc) return D3D11_RESOURCE_DIMENSION_UNKNOWN;

			D3D11_RESOURCE_DIMENSION type;
			m_rsc->GetType(&type);
			return type;
		}
		
		ID3D11ShaderResourceView* getSRV(uint32 arrayIndex, uint32 arrayCount, ImageType type);
		ID3D11RenderTargetView* getRTV(uint32 arrayIndex);
		ID3D11DepthStencilView* getDSV(uint32 arrayIndex);

		void reset()
		{
			//clear views
			m_srvCache.clear();
			m_rtvCache.clear();
			m_dsvCache.clear();
			//clear resourc
			m_rsc.Reset();
		}
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////
