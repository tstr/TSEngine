/*
    Low level render api interfaces
*/

#pragma once

#include <tscore/system/memory.h>
#include <tscore/ptr.h>

#include "Defs.h"

namespace ts
{
    enum class RenderDeviceID
    {
        NONE,
        D3D11
    };

    /*
        Device interface
        
        Reponsible for allocating/managing resources and submitting command contexts
    */
	struct RenderDevice
    {
		struct Deleter
		{
			void operator()(RenderDevice* b) { RenderDevice::destroy(b); }
		};

		using Ptr = UPtr<RenderDevice, Deleter>;

		//Initialization
		static Ptr create(RenderDeviceID id, const RenderDeviceConfig& config);
		static void destroy(RenderDevice* device);

		virtual RenderContext* getContext() = 0;
		virtual void execute(RenderContext* context) = 0;
        
        //Display methods
		virtual void setDisplayConfiguration(const DisplayConfig& displayCfg) = 0;
		virtual void getDisplayConfiguration(DisplayConfig& displayCfg) = 0;
		virtual ResourceHandle getDisplayTarget() = 0;
		
        //Query device
		virtual void queryStats(RenderStats& stats) = 0;
		virtual void queryInfo(DeviceInfo& info) = 0;
        
        //Resources
        virtual ResourceHandle createEmptyResource(ResourceHandle recycle = (ResourceHandle)0) = 0;
        virtual ResourceHandle createResourceBuffer(const ResourceData& data, const BufferResourceInfo& info, ResourceHandle recycle = (ResourceHandle)0) = 0;
		virtual ResourceHandle createResourceImage(const ResourceData* data, const ImageResourceInfo& info, ResourceHandle recycle = (ResourceHandle)0) = 0;
        //Resource set
        virtual ResourceSetHandle createResourceSet(const ResourceSetInfo& info, ResourceSetHandle recycle = (ResourceSetHandle)0) = 0;
		//Pipeline state
        virtual ShaderHandle createShader(const ShaderCreateInfo& info) = 0;
        virtual PipelineHandle createPipeline(ShaderHandle program, const PipelineCreateInfo& info) = 0;
		//Output target
        virtual TargetHandle createTarget(const TargetCreateInfo& info, TargetHandle recycle = (TargetHandle)0) = 0;
        //Commands
        virtual CommandHandle createCommand(const DrawCommandInfo& cmd, CommandHandle recycle = (CommandHandle)0) = 0;
        
        //Destroy device objects
		virtual void destroy(ResourceHandle rsc) = 0;
		virtual void destroy(ResourceSetHandle set) = 0;
		virtual void destroy(ShaderHandle shader) = 0;
		virtual void destroy(PipelineHandle state) = 0;
		virtual void destroy(TargetHandle pass) = 0;
        virtual void destroy(CommandHandle cmd) = 0;
    };
    
    /*
        Context interface
        
        Responsible for recording commands
    */
    struct RenderContext
    {
		virtual void resourceUpdate(ResourceHandle rsc, const void* memory, uint32 index = 0) = 0;
		virtual void resourceCopy(ResourceHandle src, ResourceHandle dest) = 0;
		virtual void imageResolve(ResourceHandle src, ResourceHandle dest) = 0;
		
		virtual void clearColourTarget(TargetHandle pass, uint32 colour) = 0;
		virtual void clearDepthTarget(TargetHandle pass, float depth) = 0;
		
		virtual void submit(CommandHandle command) = 0;
		
		virtual void finish() = 0;
    };
}

