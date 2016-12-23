/*
	Render API
	
	Draw command structures
*/

#include "base.h"

namespace ts
{
	class D3D11DrawCommand
	{
	public:

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		//Pipeline states
		ComPtr<ID3D11BlendState> blendState;
		ComPtr<ID3D11RasterizerState> rasterState;
		ComPtr<ID3D11DepthStencilState> depthState;
		
		//Shaders
		ComPtr<ID3D11VertexShader> shaderVertex;
		ComPtr<ID3D11PixelShader> shaderPixel;
		ComPtr<ID3D11GeometryShader> shaderGeometry;
		ComPtr<ID3D11HullShader> shaderHull;
		ComPtr<ID3D11DomainShader> shaderDomain;
		
		//Textures
		ComPtr<ID3D11ShaderResourceView> shaderResourceViews[SDrawCommand::eMaxTextureSlots];
		ComPtr<ID3D11SamplerState> shaderSamplerStates[SDrawCommand::eMaxTextureSamplerSlots];
		
		//Buffers
		ComPtr<ID3D11Buffer> indexBuffer;
		ComPtr<ID3D11Buffer> constantBuffers[SDrawCommand::eMaxConstantBuffers];
		ComPtr<ID3D11Buffer> vertexBuffers[SDrawCommand::eMaxVertexBuffers];
		uint32 vertexStrides[SDrawCommand::eMaxVertexBuffers];
		uint32 vertexOffsets[SDrawCommand::eMaxVertexBuffers];
		
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

		//static helper methods
		static HDrawCmd downcast(D3D11DrawCommand* drawCommand)
		{
			return (HDrawCmd)reinterpret_cast<HDrawCmd&>(drawCommand);
		}
		
		static D3D11DrawCommand* upcast(HDrawCmd hDraw)
		{
			return reinterpret_cast<D3D11DrawCommand*>(hDraw);
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	};
}