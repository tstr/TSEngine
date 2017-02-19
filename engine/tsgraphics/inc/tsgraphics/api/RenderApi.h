/*
	Render API
	
	The render api acts as a layer between the rendering module and the low level graphics implementation
*/

#pragma once

#include "RenderDef.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	struct IRender;
	struct IRenderContext;
	struct IAdapterFactory;

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//	Render API
	/////////////////////////////////////////////////////////////////////////////////////////////////
	struct IRender
	{
		//Resource methods
		virtual ERenderStatus createResourceBuffer(HBuffer& rsc, const SBufferResourceData& data) = 0;
		virtual ERenderStatus createResourceTexture(HTexture& rsc, const STextureResourceData* data, const STextureResourceDesc& desc) = 0;
		virtual ERenderStatus createShader(HShader& shader, const void* bytecode, uint32 bytecodesize, EShaderStage stage) = 0;
		virtual ERenderStatus createTarget(HTarget& target, const STargetDesc& desc) = 0;
		
		virtual void destroyBuffer(HBuffer buffer) = 0;
		virtual void destroyTexture(HTexture texture) = 0;
		virtual void destroyShader(HShader shader) = 0;
		virtual void destroyTarget(HTarget target) = 0;
		
		//Command methods
		virtual ERenderStatus createDrawCommand(HDrawCmd& cmd, const SDrawCommand& desc) = 0;
		virtual void destroyDrawCommand(HDrawCmd cmd) = 0;
		
		//Render context
		virtual void createContext(IRenderContext** context) = 0;
		virtual void destroyContext(IRenderContext* context) = 0;
		
		//Display methods
		virtual void setDisplayConfiguration(const SDisplayConfig& displayCfg) = 0;
		virtual void getDisplayConfiguration(SDisplayConfig& displayCfg) = 0;
		virtual void getDisplayTarget(HTarget& target) = 0;
		
		virtual void getDrawStatistics(SRenderStatistics& stats) = 0;
		
		virtual void drawBegin(const Vector& vec) = 0;
		virtual void drawEnd(IRenderContext** contexts, uint32 numContexts) = 0;
	};

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//	Render Context
	/////////////////////////////////////////////////////////////////////////////////////////////////
	struct IRenderContext
	{
		virtual void bufferUpdate(HBuffer rsc, const void* memory) = 0;
		virtual void bufferCopy(HBuffer src, HBuffer dest) = 0;
		virtual void textureUpdate(HTexture rsc, uint32 index, const void* memory) = 0;
		virtual void textureCopy(HTexture src, HTexture dest) = 0;
		virtual void textureResolve(HTexture src, HTexture dest) = 0;
		
		virtual void clearRenderTarget(HTarget target, const Vector& vec) = 0;
		virtual void clearDepthTarget(HTarget target, float depth) = 0;
		
		virtual void draw(
			HTarget target,
			const SViewport& viewport,
			const SViewport& scissor,
			HDrawCmd command
		) = 0;
		
		virtual void finish() = 0;
	};
	
	struct IAdapterFactory
	{	
		virtual uint32 getAdapterCount() const = 0;
		virtual bool enumAdapter(uint32 idx, SRenderAdapterDesc& desc) const = 0;
	};
	
	/////////////////////////////////////////////////////////////////////////////////////////////////
	
	namespace abi
	{
		extern "C"
		{
			int createRenderApi(IRender** api, const SRenderApiConfig& cfg);
			void destroyRenderApi(IRender* api);
			
			int createAdapterFactory(IAdapterFactory** adapterFactory);
			void destroyAdapterFactory(IAdapterFactory* adapterFactory);
		}
	}
	
	/////////////////////////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////////////////////////