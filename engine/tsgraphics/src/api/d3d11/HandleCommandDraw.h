/*
	Render API
	
	Draw command structures
*/

#include "base.h"
#include "handle.h"

namespace ts
{
	class D3D11DrawCommand : public Handle<D3D11DrawCommand, HDrawCmd>
	{
	public:

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		//Pipeline states
		ID3D11BlendState* blendState			= nullptr;
		ID3D11RasterizerState* rasterState		= nullptr;
		ID3D11DepthStencilState* depthState		= nullptr;
		
		//Shaders
		ID3D11VertexShader* shaderVertex		= nullptr;
		ID3D11PixelShader* shaderPixel			= nullptr;
		ID3D11GeometryShader* shaderGeometry	= nullptr;
		ID3D11HullShader* shaderHull			= nullptr;
		ID3D11DomainShader* shaderDomain		= nullptr;
		
		//Textures
		ID3D11ShaderResourceView* shaderResourceViews[SDrawCommand::eMaxTextureSlots] = { nullptr };
		ID3D11SamplerState* shaderSamplerStates[SDrawCommand::eMaxTextureSamplerSlots] = { nullptr };
		
		//Buffers
		ID3D11Buffer* indexBuffer = nullptr;
		ID3D11Buffer* constantBuffers[SDrawCommand::eMaxConstantBuffers] = { nullptr };
		ID3D11Buffer* vertexBuffers[SDrawCommand::eMaxVertexBuffers] = { nullptr };
		uint32 vertexStrides[SDrawCommand::eMaxVertexBuffers] = { 0 };
		uint32 vertexOffsets[SDrawCommand::eMaxVertexBuffers] = { 0 };
		
		ComPtr<ID3D11InputLayout> inputLayout;
		D3D11_PRIMITIVE_TOPOLOGY topology;
		
		uint32 indexStart = 0;
		uint32 indexCount = 0;
		uint32 vertexStart = 0;
		uint32 vertexCount = 0;
		int32 vertexBase = 0;
		uint32 instanceCount = 1;

		EDrawMode mode;

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	};
}