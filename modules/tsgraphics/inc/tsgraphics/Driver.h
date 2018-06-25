/*
    Low level render api interfaces
*/

#pragma once

#include <tscore/system/memory.h>
#include <tscore/ptr.h>

#include "Defs.h"

namespace ts
{
    /*
        Resource handle object
        manages lifetime of device resouces
    */
    template<typename Handle>
    class RPtr
    {
    public:
        
        RPtr() : m_d(nullptr), m_h((Handle)0) {}
        RPtr(RenderDevice* d, Handle h) : m_d(d), m_h(h) {}
        ~RPtr() { reset(); }
        
		//Moveable
        RPtr(RPtr<Handle>&& rhs)
		{
			std::swap(m_d, rhs.m_d);
			std::swap(m_h, rhs.m_h);
		}

        RPtr<Handle>& operator=(RPtr<Handle>&& rhs)
        {
            std::swap(m_d, rhs.m_d);
            std::swap(m_h, rhs.m_h);
            return *this;
        }

		//no copy
		RPtr(const RPtr<Handle>&) = delete;
		void operator=(const RPtr<Handle>&) = delete;

        RenderDevice* const device() const { return m_d; }
        Handle handle() const { return m_h; }
		Handle release() { auto h = m_h; m_h = Handle(); return h; }

		bool null() const { return (m_d == nullptr) || (m_h == Handle()); }
		operator bool() const { return !null(); }

		void reset(RenderDevice* d = nullptr, Handle h = Handle())
		{
			if (!null())
			{
				m_d->destroy(m_h);
			}

			m_d = d;
			m_h = h;
		}
        
    private:
        
        RenderDevice* m_d; //device
        Handle m_h;        //device resource handle
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
		static Ptr create(RenderDriverID id, const RenderDeviceConfig& config);
		static void destroy(RenderDevice* device);

		virtual RenderContext* context() = 0;
		virtual void commit() = 0;

        //Display methods
		virtual void setDisplayConfiguration(const DisplayConfig& displayCfg) = 0;
		virtual void getDisplayConfiguration(DisplayConfig& displayCfg) = 0;
		virtual ResourceHandle getDisplayTarget() = 0;
		
        //Query device
		virtual void queryStats(RenderStats& stats) = 0;
		virtual void queryInfo(RenderDeviceInfo& info) = 0;
        
        //Resources
        virtual RPtr<ResourceHandle> createEmptyResource(ResourceHandle recycle = (ResourceHandle)0) = 0;
        virtual RPtr<ResourceHandle> createResourceBuffer(const ResourceData& data, const BufferResourceInfo& info, ResourceHandle recycle = (ResourceHandle)0) = 0;
		virtual RPtr<ResourceHandle> createResourceImage(const ResourceData* data, const ImageResourceInfo& info, ResourceHandle recycle = (ResourceHandle)0) = 0;
        //Resource set
        virtual RPtr<ResourceSetHandle> createResourceSet(const ResourceSetCreateInfo& info, ResourceSetHandle recycle = (ResourceSetHandle)0) = 0;
		//Pipeline state
        virtual RPtr<ShaderHandle> createShader(const ShaderCreateInfo& info) = 0;
        virtual RPtr<PipelineHandle> createPipeline(ShaderHandle program, const PipelineCreateInfo& info) = 0;
		//Output target
        virtual RPtr<TargetHandle> createTarget(const TargetCreateInfo& info, TargetHandle recycle = (TargetHandle)0) = 0;
        //Commands
        virtual RPtr<CommandHandle> createCommand(const DrawCommandInfo& cmd, CommandHandle recycle = (CommandHandle)0) = 0;
        
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
		virtual void imageResolve(ResourceHandle src, ResourceHandle dest, uint32 index = 0) = 0;
		
		virtual void clearColourTarget(TargetHandle pass, uint32 colour) = 0;
		virtual void clearDepthTarget(TargetHandle pass, float depth) = 0;
		
		virtual void submit(CommandHandle command) = 0;
		
		virtual void finish() = 0;
    };
}

