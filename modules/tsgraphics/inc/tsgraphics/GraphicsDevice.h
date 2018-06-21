/*
    Low level render api interfaces
*/

#pragma once

#include <tscore/system/memory.h>
#include <tscore/ptr.h>

#include "GraphicsDefs.h"

namespace ts
{

    class GraphicsContext;
    
    /*
        Device class
        
        Reponsible for allocating resources and executing command contexts
    */
    class GraphicsDevice
    {
    public:
        
        GraphicsContext* getContext();
        
        //Display methods
		virtual void setDisplayConfiguration(const DisplayConfig& displayCfg) = 0;
		virtual void getDisplayConfiguration(DisplayConfig& displayCfg) = 0;
		virtual ResourceHandle getDisplayTarget() = 0;
		
		virtual void getRenderStats(RenderStats& stats) = 0;
		virtual void getDeviceInfo(DeviceInfo& info) = 0;
        
        //Resources
        virtual ResourceHandle createEmptyResource(ResourceHandle recycle = (ResourceHandle)0) = 0;
        virtual ResourceHandle createResourceBuffer(const ResourceData& data, const BufferResourceInfo& info, ResourceHandle recycle = (ResourceHandle)0) = 0;
		virtual ResourceHandle createResourceImage(const ResourceData* data, const ImageResourceInfo& info, ResourceHandle recycle = (ResourceHandle)0) = 0;
        virtual ResourceSetHandle createResourceSet(const ResourceSetInfo& info, ResourceSetHandle recycle = (ResourceSetHandle)0) = 0;
		virtual ShaderHandle createShader(const ShaderCreateInfo& info) = 0;
        virtual StateHandle createState(ShaderHandle program, const StateCreateInfo& info);
		virtual TargetHandle createTarget(const TargetCreateInfo& info, TargetHandle recycle = (TargetHandle)0) = 0;
		
		virtual void destroy(ResourceHandle rsc) = 0;
		virtual void destroy(ResourceSetHandle set) = 0;
		virtual void destroy(ShaderHandle shader) = 0;
		virtual void destroy(StateHandle state) = 0;
		virtual void destroy(TargetHandle pass) = 0;
        
        //Initialization
        static UPtr<GraphicsDevice> create(const GraphicsDeviceConfig& config);
        static void destroy(GraphicsDevice* device);
    };
    
    /*
        Context class
        
        Responsible for recording commands
    */
    class GraphicsContext
    {
		virtual void resourceUpdate(ResourceHandle rsc, const void* memory, uint32 index = 0) = 0;
		virtual void resourceCopy(ResourceHandle src, ResourceHandle dest) = 0;
		virtual void imageResolve(ResourceHandle src, ResourceHandle dest) = 0;
		
		virtual void clearColourTarget(TargetHandle pass, uint32 colour) = 0;
		virtual void clearDepthTarget(TargetHandle pass, float depth) = 0;
		
		virtual void draw(const DrawCommand& command) = 0;
		
		virtual void finish() = 0;
    };
    
    
    
    
    
}

