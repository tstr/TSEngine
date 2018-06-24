/*
	Shader object
*/

#include <fstream>

#include <tsgraphics/Shader.h>
#include <tsgraphics/schemas/Shader.rcs.h>

using namespace ts;

using std::ifstream;
using std::ios;

///////////////////////////////////////////////////////////////////////////////

inline constexpr uint32 fmtSig(char a, char b, char c, char d)
{
	return (d << 24) | (c << 16) | (b << 8) | (a << 0);
}

enum Signatures : uint32
{
	TSSH = fmtSig('T', 'S', 'S', 'H'),

	SVTX = fmtSig('S', 'V', 'T', 'X'),
	SGEO = fmtSig('S', 'G', 'E', 'O'),
	STEC = fmtSig('S', 'T', 'E', 'C'),
	STEV = fmtSig('S', 'T', 'E', 'V'),
	SPIX = fmtSig('S', 'P', 'I', 'X')
};

#define CHECK_SIGN(signature, compare) if (signature != signature) { return false; }

///////////////////////////////////////////////////////////////////////////////

bool ShaderProgram::load(RenderDevice* device, const String& shaderFile)
{
	rc::ResourceLoader loader(ifstream(shaderFile, ios::binary));

	if (loader.fail())
	{
		return false;
	}

	tsr::Shader& shaderRsc = loader.deserialize<tsr::Shader>();

	//Verify signature
	CHECK_SIGN(shaderRsc.signature(), TSSH)

	ShaderCreateInfo shaderInf;

	//Vertex shader code
	if (shaderRsc.has_vertex())
	{
		ShaderBytecode& bc = shaderInf.stages[(size_t)ShaderStage::VERTEX];
		const auto& stage = shaderRsc.vertex();
		CHECK_SIGN(stage.signature(), SVTX)

		//HLSL bytecode
		const auto& hlsl = stage.code_hlslSM5();
		bc.bytecode = hlsl.data();
		bc.size = hlsl.size();
	}

	//Geometry shader code
	if (shaderRsc.has_geometry())
	{
		ShaderBytecode& bc = shaderInf.stages[(size_t)ShaderStage::GEOMETRY];
		const auto& stage = shaderRsc.geometry();
		CHECK_SIGN(stage.signature(), SGEO)

		//HLSL bytecode
		const auto& hlsl = stage.code_hlslSM5();
		bc.bytecode = hlsl.data();
		bc.size = hlsl.size();
	}

	//Tessellation control shader code
	if (shaderRsc.has_tessControl())
	{
		ShaderBytecode& bc = shaderInf.stages[(size_t)ShaderStage::TESSCTRL];
		const auto& stage = shaderRsc.tessControl();
		CHECK_SIGN(stage.signature(), STEC)

		//HLSL bytecode
		const auto& hlsl = stage.code_hlslSM5();
		bc.bytecode = hlsl.data();
		bc.size = hlsl.size();
	}

	//Tessellation evaluation shader code
	if (shaderRsc.has_tessEval())
	{
		ShaderBytecode& bc = shaderInf.stages[(size_t)ShaderStage::TESSEVAL];
		const auto& stage = shaderRsc.tessEval();
		CHECK_SIGN(stage.signature(), STEV)

		//HLSL bytecode
		const auto& hlsl = stage.code_hlslSM5();
		bc.bytecode = hlsl.data();
		bc.size = hlsl.size();
	}

	//Pixel shader code
	if (shaderRsc.has_pixel())
	{
		ShaderBytecode& bc = shaderInf.stages[(size_t)ShaderStage::PIXEL];
		const auto& stage = shaderRsc.pixel();
		CHECK_SIGN(stage.signature(), SPIX)

		//HLSL bytecode
		const auto& hlsl = stage.code_hlslSM5();
		bc.bytecode = hlsl.data();
		bc.size = hlsl.size();
	}

	//Create device resource
	m_program = device->createShader(shaderInf);

	return !m_program.null();
}

///////////////////////////////////////////////////////////////////////////////
