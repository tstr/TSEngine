/*
	Buffer objects
*/

#pragma once

#include "Driver.h"

namespace ts
{
	/*
		Simple buffer class
	*/
	class Buffer : private RPtr<ResourceHandle>
	{
	public:

		using Base = RPtr<ResourceHandle>;

		using Base::handle;
		using Base::device;
		using Base::null;
		using Base::operator bool;

		Buffer() {}
		Buffer(Buffer&& rhs) : Base(std::move((Base&&)*this)) {}
		Buffer& operator=(Buffer&& rhs) { (Base&)*this = (Base&&)rhs; return *this; }

		/*
			Construct an empty staging buffer with a reserved handle
		*/
		Buffer(RenderDevice* dev)
		{
			((Base&)*this) = dev->createEmptyResource();
		}
		
		/*
			Construct a buffer
		*/
		Buffer(RenderDevice* dev, const ResourceData& data, const BufferResourceInfo& info)
		{
			((Base&)*this) = dev->createResourceBuffer(data, info);
		}

		/*
			Create a buffer a given pointer to memory
		*/
		static Buffer create(RenderDevice* device, const void* ptr, uint32 size, BufferType type)
		{
			ResourceData d;
			d.memory = ptr;
			BufferResourceInfo i;
			i.size = size;
			i.type;

			return Buffer(device, d, i);
		}

		/*
			Create a buffer from a typed object
		*/
		template<
			typename StructType
			//typename = std::enable_if<std::is_pod<StructType>::value>::type
		>
		static Buffer create(RenderDevice* device, const StructType& data, BufferType type)
		{
			return Buffer::create(device, (const void*)&data, sizeof(StructType), type);
		}
	};
}
