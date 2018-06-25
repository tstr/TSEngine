/*
	Render API
	
	Render context
*/

#pragma once

#include "Base.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	class Dx11;

	class Dx11Context : public RenderContext
	{
	private:

		Dx11* m_driver;
		ComPtr<ID3D11DeviceContext> m_context;
		ComPtr<ID3D11CommandList> m_contextCommandList;

	public:
		
		Dx11Context() {}
		Dx11Context(Dx11* api);
		~Dx11Context();

		void resourceUpdate(ResourceHandle rsc, const void* memory, uint32 index) override;
		void resourceCopy(ResourceHandle src, ResourceHandle dest) override;
		void imageResolve(ResourceHandle src, ResourceHandle dest, uint32 index) override;

		void clearColourTarget(TargetHandle pass, uint32 colour) override;
		void clearDepthTarget(TargetHandle pass, float depth) override;

		void submit(CommandHandle command) override;

		void finish() override;

		void resetCommandList();
		ID3D11CommandList* getCommandList() const { return m_contextCommandList.Get(); }
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
