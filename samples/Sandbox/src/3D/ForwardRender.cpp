/*
    Forward rendering pipeline
*/

#include <tscore/debug/assert.h>

#include "ForwardRender.h"

using namespace std;
using namespace ts;

///////////////////////////////////////////////////////////////////////////////

ForwardRenderer::ForwardRenderer(GraphicsSystem* gfx) :
	m_gfx(gfx)
{
	tsassert(m_gfx);

	RenderDevice* device = m_gfx->device();

	m_perScene = Buffer::create(device, SceneConstants(), BufferType::CONSTANTS);
	m_perMesh  = Buffer::create(device, MeshConstants(),  BufferType::CONSTANTS);

	tsassert(m_perScene);
	tsassert(m_perMesh);

	preloadShaders();
}

///////////////////////////////////////////////////////////////////////////////

void ForwardRenderer::preloadShaders()
{
	RenderDevice* device = m_gfx->device();
	
	Path standard(m_gfx->getRootPath());
	standard.addDirectories("shaderbin/Standard.shader");

	tsassert(m_shader.load(device, standard.str()));
}

///////////////////////////////////////////////////////////////////////////////

Renderable ForwardRenderer::createRenderable(const MeshInfo& mesh, const MaterialCreateInfo& info)
{
	tsassert(m_gfx);

	RenderDevice* device = m_gfx->device();

	Renderable item;

	//Create material 
	loadMaterial(item.mat, info);

	//Item parameters
	item.resources = makeResourceSet(mesh.data, item.mat);
	item.pso = makePipeline(mesh, item.mat);

	//Command arguments
	if (mesh.data.mode == DrawMode::VERTEX || mesh.data.mode == DrawMode::INSTANCED)
	{
		item.params.start = mesh.data.vertexStart;
		item.params.count = mesh.data.vertexCount;
		item.params.instances = 1;
		item.params.vbase = mesh.data.vertexBase;
	}
	else //indexed
	{
		item.params.start = mesh.data.indexStart;
		item.params.count = mesh.data.indexCount;
		item.params.instances = 1;
		item.params.vbase = mesh.data.vertexBase;
	}

	item.params.mode = mesh.data.mode;

	return move(item);
}

void ForwardRenderer::loadMaterial(MaterialInstance& mat, const MaterialCreateInfo& info)
{
	RenderDevice* device = m_gfx->device();

	//Create material buffer
	mat.buffer = Buffer::create(device, info.constants, BufferType::CONSTANTS);

	//Load material images
	for (const auto& e : {
		make_pair(BIND_DIFFUSE_MAP,      "diffuseMap"),
		make_pair(BIND_NORMAL_MAP,       "normalMap"),
		make_pair(BIND_SPECULAR_MAP,     "specularMap"),
		make_pair(BIND_DISPLACEMENT_MAP, "displacementMap")
	})
	{
		ImageBindings imgBinding = e.first;
		const char*   imgMapping = e.second;

		auto it = info.images.find(imgMapping);
		
		//If an imagepath has been specified for this mapping
		if (it != info.images.end())
		{
			//Load it
			Image img(device, it->second.str());

			mat.images[imgBinding] = img.getView2D(0);

			m_imageCache.push_back(move(img));
		}
	}
}

RPtr<ResourceSetHandle> ForwardRenderer::makeResourceSet(const Mesh& mesh, const MaterialInstance& mat)
{
	RenderDevice* device = m_gfx->device();

	ResourceSetCreateInfo info;

	//Constant buffer resources
	BindingSet<ResourceHandle> constantBuffers;
	constantBuffers[BIND_SCENE_CONSTANTS] = m_perScene.handle();
	constantBuffers[BIND_MESH_CONSTANTS] = m_perMesh.handle();
	constantBuffers[BIND_MAT_CONSTANTS] = mat.buffer.handle();

	info.constantBuffers = constantBuffers.data();
	info.constantBuffersCount = (uint32)constantBuffers.count();

	//Image resources
	info.resources = mat.images.data();
	info.resourceCount = (uint32)mat.images.count();

	//Mesh resources
	VertexBufferView vbv;
	vbv.buffer = mesh.vertices;
	vbv.offset = 0;
	vbv.stride = mesh.vertexStride;
	info.vertexBuffers = &vbv;
	info.vertexBufferCount = 1;
	info.indexBuffer = mesh.indices;

	return device->createResourceSet(info);
}

void findAttribute(const char* semantic, VertexAttributeType type, const VertexAttributeMap* attribMap, vector<VertexAttribute>& attribs)
{
	auto it = attribMap->find(semantic);
	if (it != attribMap->end())
	{
		if ((string)semantic == "TEXCOORD0") semantic = "TEXCOORD";
		if ((string)semantic == "COLOUR0") semantic = "COLOUR";

		VertexAttribute sid;
		sid.bufferSlot = 0;
		sid.byteOffset = it->second;
		sid.channel = VertexAttributeChannel::VERTEX;
		sid.semanticName = semantic;
		sid.type = type;
		attribs.push_back(sid);
	}
}

RPtr<PipelineHandle> ForwardRenderer::makePipeline(const MeshInfo& mesh, const MaterialInstance& mat)
{
	RenderDevice* device = m_gfx->device();

	//Image samplers
	BindingSet<SamplerState> samplers;
	samplers[0].filtering = ImageFilterMode::ANISOTROPIC;
	samplers[0].addressU = samplers[0].addressV = samplers[0].addressW = ImageAddressMode::WRAP;
	samplers[0].anisotropy = 8;

	PipelineCreateInfo pso;
	//Samplers
	pso.samplers = samplers.data();
	pso.samplerCount = samplers.count();

	//States
	pso.blend.enable = false;
	pso.depth.enableDepth = true;
	pso.depth.enableStencil = false;
	pso.raster.enableScissor = false;
	pso.raster.cullMode = CullMode::BACK;
	pso.raster.fillMode = FillMode::SOLID;

	//Vertex layout
	vector<VertexAttribute> attrib;

	findAttribute("POSITION",  VertexAttributeType::FLOAT4, mesh.attributeMap, attrib);
	findAttribute("TEXCOORD0", VertexAttributeType::FLOAT2, mesh.attributeMap, attrib);
	findAttribute("COLOUR0",   VertexAttributeType::FLOAT4, mesh.attributeMap, attrib);
	findAttribute("NORMAL",    VertexAttributeType::FLOAT3, mesh.attributeMap, attrib);
	findAttribute("TANGENT",   VertexAttributeType::FLOAT3, mesh.attributeMap, attrib);
	findAttribute("BITANGENT", VertexAttributeType::FLOAT3, mesh.attributeMap, attrib);

	pso.vertexAttributeCount = attrib.size();
	pso.vertexAttributeList = attrib.data();
	
	//Vertex topology
	pso.topology = mesh.topology;

	return device->createPipeline(m_shader.handle(), pso);
}

///////////////////////////////////////////////////////////////////////////////
//	Draw submission
///////////////////////////////////////////////////////////////////////////////

void ForwardRenderer::draw(const Renderable& item, const Matrix& transform)
{
	tsassert(m_gfx);
	auto ctx = m_gfx->device()->context();
	
	MeshConstants constants;
	constants.world = transform.transpose();
	ctx->resourceUpdate(m_perMesh.handle(), &constants);
	
	ctx->draw(
		m_targets,
		item.pso.handle(),
		item.resources.handle(),
		item.params
	);
}


void ForwardRenderer::begin(ForwardRenderTarget& targets)
{
	m_targets = targets.handle();

	tsassert(m_gfx);
	tsassert(m_targets != TargetHandle());

	auto ctx = m_gfx->device()->context();

	SceneConstants constants;

	//Ambient light
	constants.ambient = m_ambientColour;
	constants.view = m_viewMatrix;
	constants.projection = m_projMatrix;
	//constants.viewPos = constants.view.inverse().getTranslation();

	//Directional light
	constants.direct.colour = m_directLightColour;
	constants.direct.dir = m_directLightDir;
	constants.direct.dir.w() = 0.0f;
	constants.direct.dir.normalize();
	constants.direct.dir = Matrix::transform4D(constants.direct.dir, constants.view);
	constants.direct.dir.normalize();

	//Dynamic lighting
	for (int i = 0; i < 4; i++)
	{
		constants.dynamic[i] = m_dynamicLights[i];
		constants.dynamic[i].pos.w() = 1.0f;
		constants.dynamic[i].pos = Matrix::transform4D(constants.dynamic[i].pos, constants.view);
	}

	Matrix::transpose(constants.view);
	Matrix::transpose(constants.projection);

	//Update scene constants
	ctx->resourceUpdate(m_perScene.handle(), &constants);

	//Clear backbuffer
	ctx->clearDepthTarget(m_targets, 1.0f);
	ctx->clearColourTarget(m_targets, RGBA(80, 166, 100));
}

void ForwardRenderer::end()
{
	// do nothing
}

///////////////////////////////////////////////////////////////////////////////
