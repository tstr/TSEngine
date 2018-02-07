/*
	Render API

	D3D11Shader class
*/

#include "render.h"
#include "handle.h"

namespace ts
{
	class D3D11Shader : public Handle<D3D11Shader, HShader>
	{
	private:
		
		ComPtr<ID3D11DeviceChild> m_shaderInterface;
		EShaderStage m_shaderStage;
		MemoryBuffer m_shaderBytecode;
		
	public:
		
		D3D11Shader(ID3D11VertexShader* s, MemoryBuffer&& buf) :
			m_shaderInterface((ID3D11DeviceChild*)s),
			m_shaderStage(eShaderStageVertex),
			m_shaderBytecode(buf)
		{}

		D3D11Shader(ID3D11PixelShader* s, MemoryBuffer&& buf) :
			m_shaderInterface((ID3D11DeviceChild*)s),
			m_shaderStage(eShaderStagePixel),
			m_shaderBytecode(buf)
		{}

		D3D11Shader(ID3D11GeometryShader* s, MemoryBuffer&& buf) :
			m_shaderInterface((ID3D11DeviceChild*)s),
			m_shaderStage(eShaderStageGeometry),
			m_shaderBytecode(buf)
		{}

		D3D11Shader(ID3D11HullShader* s, MemoryBuffer&& buf) :
			m_shaderInterface((ID3D11DeviceChild*)s),
			m_shaderStage(eShaderStageTessCtrl),
			m_shaderBytecode(buf)
		{}

		D3D11Shader(ID3D11DomainShader* s, MemoryBuffer&& buf) :
			m_shaderInterface((ID3D11DeviceChild*)s),
			m_shaderStage(eShaderStageTessEval),
			m_shaderBytecode(buf)
		{}

		D3D11Shader(ID3D11ComputeShader* s, MemoryBuffer&& buf) :
			m_shaderInterface((ID3D11DeviceChild*)s),
			m_shaderStage(eShaderStageCompute),
			m_shaderBytecode(buf)
		{}

		~D3D11Shader() {}

		ID3D11DeviceChild* getShader() const { return m_shaderInterface.Get(); }
		EShaderStage getShaderType() const { return m_shaderStage; }

		void getShaderBytecode(void** bytecode, uint32& bytecodesize)
		{
			*bytecode = m_shaderBytecode.pointer();
			bytecodesize = (UINT)m_shaderBytecode.size();
		}
	};
}
