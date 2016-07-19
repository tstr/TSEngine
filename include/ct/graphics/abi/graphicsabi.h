/*
	Rendering abi - provides abstraction layer between renderer and underlying graphics api implementation
*/

#pragma once

#include <ct\graphics\graphics.h>

#include <ostream>
#include <istream>

#ifndef C3E_GFX_RENDER_API
#define C3E_GFX_RENDER_API
#endif

namespace C3E
{
	namespace ABI
	{
		class IRenderApi;
		class IRenderContext;

		enum ShaderCompileFlag
		{
			SHADER_COMPILE_DEBUG = 1
		};

		struct RenderConfig
		{
			uint32 w = 0;
			uint32 h = 0;
			bool windowed = true;
			Multisampling sampling;
		};

		extern "C"
		{
			C3E_GFX_RENDER_API IRenderApi* CreateRenderingInterface(const Graphics::Configuration& cfg);
			C3E_GFX_RENDER_API void DestroyRenderingInterface(IRenderApi* api);

			C3E_GFX_RENDER_API bool CompileShaderCode(
				const char* sourcefile,
				const char* shadercode,
				size_t shadercode_size,
				std::ostream& output,
				std::ostream& erroutput,
				size_t& size,
				const char* entrypoint,
				ShaderType type,
				int flags
			);
		}

		struct ShaderInputDescriptor
		{
			uint32 slot = 0;
			const char* semanticName = "";
			uint32 byteOffset = 0;
			uint32 vectorComponents = 1;
		};

		struct ShaderResourceData
		{
			const void* mem = nullptr;
			uint32 memPitch = 0; 		//Width of resource in bytes
			uint32 memSlicePitch = 0;	//Depth of resource in bytes
		};

		struct ShaderResourceDescriptor
		{
			ETextureFormat texformat = ETextureFormat::FormatUnknown;
			ETextureType textype = ETextureType::TypeTexture2D;
			EResourceUsage rscusage = EResourceUsage::UsageDefault;

			uint32 width = 0;
			uint32 height = 0;
			uint32 depth = 0;

			uint32 arraySize = 1;

			Multisampling sampling;

			bool isCubemap = false;
			bool useMips = false;
		};

		struct RenderTargetDescriptor
		{
			ETextureFormat texformat;

			uint32 width = 0;
			uint32 height = 0;

			uint32 arraySize = 1;

			Multisampling sampling;

			bool useMips = false;
			bool isCubemap = false;
		};

		struct DepthBufferDescriptor
		{
			uint32 width = 0;
			uint32 height = 0;

			uint32 arraySize = 1;

			Multisampling sampling;

			bool isCubemap = false;
		};

		class IRenderApi
		{
		public:

			///////////////////////////////////////////////////////////////////////////////////////////
			//Resource creation

			virtual ResourceHandle CreateShader(
				const byte* bytecode,
				size_t b,
				ShaderType type
				) = 0;

			virtual bool DestroyShader(ResourceHandle h) = 0;

			virtual ResourceHandle CreateShaderInputDescriptor(
				const byte* bytecode,
				size_t b,
				const ShaderInputDescriptor* sids,
				uint32 sid_num
			) = 0;

			virtual bool DestroyShaderInputDescriptor(ResourceHandle h) = 0;

			virtual ResourceHandle CreateBuffer(
				const void* mem,
				uint32 memsize,
				EResourceType rsctype,
				EResourceUsage rscusage
			) = 0;

			virtual bool DestroyBuffer(ResourceHandle h) = 0;

			virtual ResourceHandle CreateShaderResource(
				const ShaderResourceData* data,
				const ShaderResourceDescriptor& desc
			) = 0;

			virtual ResourceHandle CreateRenderTarget(
				const RenderTargetDescriptor& desc
			) = 0;

			virtual ResourceHandle CreateDepthBuffer(
				const DepthBufferDescriptor& desc
			) = 0;

			virtual bool DestroyShaderResource(ResourceHandle h) = 0;
			virtual bool DestroyRenderTarget(ResourceHandle h) = 0;
			virtual bool DestroyDepthBuffer(ResourceHandle h) = 0;

			virtual void SetResourceString(ResourceHandle h, const char* debugstring) = 0;

			///////////////////////////////////////////////////////////////////////////////////////////

			virtual void DrawBegin(const Colour& c) = 0;
			virtual void DrawEnd() = 0;

			virtual IRenderContext* CreateContext() = 0;
			virtual void DestroyContext(IRenderContext*) = 0;
			virtual void DispatchContext(IRenderContext*) = 0;

			//Set/get the viewport of the primary rendertarget
			virtual void SetViewport(const Graphics::Viewport& vp) = 0;
			virtual void GetViewport(Graphics::Viewport& vp) = 0;

			virtual ResourceHandle GetDefaultRenderTarget() const = 0;
			virtual ResourceHandle GetDefaultDepthBuffer() const = 0;

			//virtual ResourceHandle GetViewTextureArray(ResourceHandle v) const = 0;
			//virtual ResourceHandle GetViewTexture(ResourceHandle v, uint32 index) const = 0;
			virtual ResourceHandle GetViewTexture(ResourceHandle v) const = 0;

			///////////////////////////////////////////////////////////////////////////////////////////

			virtual void QueryDebugStatistics(Graphics::DebugInfo& info) = 0;

			///////////////////////////////////////////////////////////////////////////////////////////
		};

		class IRenderContext
		{
		public:

			virtual void ContextBegin() = 0;
			virtual void ContextEnd() = 0;

			virtual void ResourceSet(ResourceHandle dest, const void* mem, uint32 size) = 0;
			virtual void ResourceCopy(ResourceHandle source, ResourceHandle dest) = 0;
			virtual void ResourceResolve(ResourceHandle source, ResourceHandle dest) = 0;

			virtual void SetRenderTarget(ResourceHandle renderTarget, ResourceHandle depthBuffer, uint32 depthBufferIndex = 0, uint32 renderTargetIndex = 0) = 0;
			virtual void ClearRenderTarget(ResourceHandle renderTarget, const Colour& c) = 0;
			virtual void ResizeRenderTarget(ResourceHandle renderTarget, uint32 w, uint32 h) const = 0;
			virtual void ResizeDepthBuffer(ResourceHandle depthBuffer, uint32 w, uint32 h) const = 0;

			virtual void EnableAlphaBlending(bool e) = 0;

			//virtual void DrawRenderBlock(const RenderBlock& block) = 0;
			//virtual void DrawComputeBlock(const ComputeBlock& block) = 0;

			virtual void SetShader(ResourceHandle h, ShaderType type) = 0;
			virtual void SetShaderInputDescriptor(ResourceHandle h) = 0;
			virtual void SetShaderResource(uint32 index, ResourceHandle tex) = 0;

			virtual void SetVertexTopology(VertexTopology vt) = 0;

			virtual void SetIndexBuffer(ResourceHandle ibuffer, uint32 offset) = 0;
			virtual void SetVertexBuffer(uint32 index, ResourceHandle vbuffer, uint32 stride, uint32 offset) = 0;
			virtual void SetShaderBuffer(uint32 index, ResourceHandle ebuffer) = 0;

			virtual void Draw(uint32 voffset, uint32 vcount) = 0;
			//Index offset, index count, vertex offset
			virtual void DrawIndexed(uint32 ioffset, uint32 icount, uint32 voffset) = 0;
		};
	}
}

