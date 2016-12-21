/*
	Render API
	
	Render context
*/

#include "render.h"
#include "helpers.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	class D3D11RenderContext : public ts::IRenderContext
	{
	private:

		D3D11Render* m_api;
		ComPtr<ID3D11DeviceContext> m_context;
		ComPtr<ID3D11CommandList> m_contextCommandList;

	public:
		
		D3D11RenderContext(D3D11Render* api);
		~D3D11RenderContext();

		void bufferUpdate(HBuffer rsc, const void* memory) override;
		void bufferCopy(HBuffer src, HBuffer dest) = 0;
		void textureUpdate(HTexture rsc, uint32 index, const void* memory) override;
		void textureCopy(HTexture src, HTexture dest) override;
		void textureResolve(HTexture src, HTexture dest) override;

		void clearRenderTarget(HTarget target, const Vector& vec) override;
		void clearDepthTarget(HTarget target, float depth) override;

		void draw(
			HTarget target,
			const SViewport& viewport,
			const SViewport& scissor,
			HDrawCmd command
		) override;
		
		void finish() override;

		void resetCommandList();
		ComPtr<ID3D11CommandList> getCommandList() const { return m_contextCommandList; }
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
