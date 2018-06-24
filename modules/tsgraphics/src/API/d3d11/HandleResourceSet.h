/*
	Render API

	Resource Set object
*/

#pragma once

#include <vector>

#include "Base.h"
#include "Handle.h"
#include "HandleResource.h"

namespace ts
{
	class D3D11ResourceSet : public Handle<D3D11ResourceSet, ResourceSetHandle>
	{
	public:

		//shader resource view
		struct SRV
		{
			D3D11Resource* tex;
			ImageType type;
			uint32 index;
			uint32 count;

			SRV() : tex(nullptr), type(ImageType::_2D), index(0), count(0) {}

			SRV(const ImageView& view) :
				tex(D3D11Resource::upcast(view.image)),
				type(view.type),
				index(view.index),
				count(view.count)
			{}

			ID3D11ShaderResourceView* getView() const
			{
				return (tex == nullptr) ? nullptr : tex->getSRV(index, count, type);
			}
		};

		//vertex buffer view
		struct VBV
		{
			D3D11Resource* buffer;
			uint32 stride;
			uint32 offset;

			VBV() : buffer(nullptr), stride(0), offset(0) {}

			VBV(const VertexBufferView& view) :
				buffer(D3D11Resource::upcast(view.buffer)),
				stride(view.stride),
				offset(view.offset)
			{}

			ID3D11Buffer* getBuffer() const { return (buffer == nullptr) ? nullptr : buffer->asBuffer(); }
		};

		using CBV = D3D11Resource*;

		HRESULT create(const ResourceSetCreateInfo& info);

		void bind(ID3D11DeviceContext* context);

		void reset()
		{
			m_srvs.clear();
			m_constantBuffers.clear();
			m_vertexBuffers.clear();
			m_indexBuffer = nullptr;
		}

	private:

		std::vector<SRV> m_srvs;
		std::vector<CBV> m_constantBuffers;
		std::vector<VBV> m_vertexBuffers;
		D3D11Resource* m_indexBuffer;
	};
}