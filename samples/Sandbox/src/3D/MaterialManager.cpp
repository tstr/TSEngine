/*
    Material manager
*/

#include <tscore/strings.h>
#include <tscore/debug/assert.h>
#include <tsgraphics/Shader.h>
#include <tsgraphics/BindingSet.h>

#include "MaterialManager.h"

using namespace std;
using namespace ts;

////////////////////////////////////////////////////////////////////////////////////////////////////////

MaterialManager::MaterialManager(GraphicsSystem* gfx) :
	m_gfx(gfx)
{
	tsassert(m_gfx);
	preloadShaders();
}

void MaterialManager::preloadShaders()
{
	RenderDevice* device = m_gfx->device();

	auto load = [this, device](ShaderProgram& program, const char* path) {
		Path standard(m_gfx->getRootPath());
		standard.addDirectories(path);
		tsassert(program.load(device, standard.str()));
	};

	load(m_shader,        "shaderbin/Standard.shader");
	load(m_shaderNormMap, "shaderbin/StandardNormalMapped.shader");
	load(m_shadowMapper,  "shaderbin/ShadowMap.shader");
}

ShaderHandle MaterialManager::selectShader(const Mesh& mesh, const PhongMaterial& mat)
{
	//If material specifies a normal map
	if (mat.normalMap.image != ResourceHandle())
	{
		tsassert(mesh.vertexAttributes.find("TEXCOORD0") != mesh.vertexAttributes.end());
		tsassert(mesh.vertexAttributes.find("TANGENT") != mesh.vertexAttributes.end());

		return m_shaderNormMap.handle();
	}
	//Just use diffuse mapping
	else
	{
		tsassert(mesh.vertexAttributes.find("TEXCOORD0") != mesh.vertexAttributes.end());

		return m_shader.handle();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Pipeline creation methods
////////////////////////////////////////////////////////////////////////////////////////////////////////

RPtr<PipelineHandle> MaterialManager::getForwardPipeline(const Mesh& mesh, const PhongMaterial& mat)
{
	RenderDevice* device = m_gfx->device();

	//Image samplers
	BindingSet<SamplerState> samplers;
	samplers[0].filtering = ImageFilterMode::ANISOTROPIC;
	samplers[0].addressU = samplers[0].addressV = samplers[0].addressW = ImageAddressMode::WRAP;
	samplers[0].anisotropy = 8;

	samplers[1].filtering = ImageFilterMode::ANISOTROPIC;
	samplers[1].addressU = samplers[1].addressV = samplers[1].addressW = ImageAddressMode::CLAMP;
	samplers[1].anisotropy = 8;

	PipelineCreateInfo pso;
	//Samplers
	pso.samplers = samplers.data();
	pso.samplerCount = samplers.count();

	//States
	pso.blend.enable = mat.enableAlpha;
	pso.depth.enableDepth = true;
	pso.depth.enableStencil = false;
	pso.raster.enableScissor = false;
	pso.raster.cullMode = CullMode::BACK;
	pso.raster.fillMode = FillMode::SOLID;

	//Vertex layout
	vector<VertexAttribute> attrib;

	findAttribute("POSITION", VertexAttributeType::FLOAT4, mesh.vertexAttributes, attrib);
	findAttribute("TEXCOORD0", VertexAttributeType::FLOAT2, mesh.vertexAttributes, attrib);
	findAttribute("COLOUR0", VertexAttributeType::FLOAT4, mesh.vertexAttributes, attrib);
	findAttribute("NORMAL", VertexAttributeType::FLOAT3, mesh.vertexAttributes, attrib);
	findAttribute("TANGENT", VertexAttributeType::FLOAT3, mesh.vertexAttributes, attrib);
	findAttribute("BITANGENT", VertexAttributeType::FLOAT3, mesh.vertexAttributes, attrib);

	pso.vertexAttributeCount = attrib.size();
	pso.vertexAttributeList = attrib.data();

	//Vertex topology
	pso.topology = mesh.vertexTopology;

	return device->createPipeline(selectShader(mesh, mat), pso);
}

RPtr<PipelineHandle> MaterialManager::getShadowPipeline(const Mesh& mesh, const PhongMaterial& material)
{
	RenderDevice* device = m_gfx->device();

	PipelineCreateInfo pso;
	//States
	pso.blend.enable = false;
	pso.depth.enableDepth = true;
	pso.depth.enableStencil = false;
	pso.raster.enableScissor = false;
	pso.raster.cullMode = CullMode::FRONT;
	pso.raster.fillMode = FillMode::SOLID;

	//Vertex layout
	vector<VertexAttribute> attrib;

	findAttribute("POSITION", VertexAttributeType::FLOAT4, mesh.vertexAttributes, attrib);
	findAttribute("TEXCOORD0", VertexAttributeType::FLOAT2, mesh.vertexAttributes, attrib);
	findAttribute("COLOUR0", VertexAttributeType::FLOAT4, mesh.vertexAttributes, attrib);
	findAttribute("NORMAL", VertexAttributeType::FLOAT3, mesh.vertexAttributes, attrib);
	findAttribute("TANGENT", VertexAttributeType::FLOAT3, mesh.vertexAttributes, attrib);
	findAttribute("BITANGENT", VertexAttributeType::FLOAT3, mesh.vertexAttributes, attrib);

	pso.vertexAttributeCount = attrib.size();
	pso.vertexAttributeList = attrib.data();

	//Vertex topology
	pso.topology = mesh.vertexTopology;

	return device->createPipeline(m_shadowMapper.handle(), pso);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

void MaterialManager::findAttribute(
	const char* semantic,
	VertexAttributeType type,
	const VertexAttributeMap& attribMap,
	std::vector<VertexAttribute>& attribs
)
{
	auto it = attribMap.find(semantic);
	if (it != attribMap.end())
	{
		if ((String)semantic == "TEXCOORD0") semantic = "TEXCOORD";
		if ((String)semantic == "COLOUR0") semantic = "COLOUR";

		VertexAttribute sid;
		sid.bufferSlot = 0;
		sid.byteOffset = it->second;
		sid.channel = VertexAttributeChannel::VERTEX;
		sid.semanticName = semantic;
		sid.type = type;
		attribs.push_back(sid);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

