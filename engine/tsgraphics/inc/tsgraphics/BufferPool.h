/*
	Buffer pool:
	
	Pool for allocating buffer resources
*/

#pragma once

#include <tsgraphics/abi.h>
#include <tsgraphics/GraphicsCore.h>

namespace ts
{
	/*
		BufferPool allocates different types of buffers,
		buffer lifetime is tied to the lifetime of the pool
	*/
	class CBufferPool
	{
	public:
		
		typedef HBuffer Handle;
		
		CBufferPool() {}
		CBufferPool(GraphicsCore* core) : m_graphics(core) {}
		~CBufferPool() { this->clear(); }
		
		/*
			Buffer creation methods
		*/
		TSGRAPHICS_API Handle createConstantBuffer(const void* data, size_t dataSize);
		TSGRAPHICS_API Handle createVertexBuffer(const void* data, size_t dataSize);
		TSGRAPHICS_API Handle createIndexBuffer(const void* data, size_t dataSize);
		
		template<typename T>
		Handle createConstantBuffer(const T& data) { return this->createConstantBuffer(&data, sizeof(T)); }
		template<typename T>
		Handle createVertexBuffer(const T& data) { return this->createVertexBuffer(&data, sizeof(T)); }
		template<typename T>
		Handle createIndexBuffer(const T& data) { return this->createIndexBuffer(&data, sizeof(T)); }
		
		/*
			Destroy a particular buffer
		*/
		TSGRAPHICS_API void destroy(Handle buffer);
		
		/*
			Destroy all loaded buffers
		*/
		TSGRAPHICS_API void clear();
		
	private:
		
		GraphicsCore* m_graphics = nullptr;
		std::vector<Handle> m_pool;
	};
}
