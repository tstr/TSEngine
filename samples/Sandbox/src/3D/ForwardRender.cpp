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

	m_perScene = Buffer::create(device, m_perSceneConst, BufferType::CONSTANTS);
	m_perMesh  = Buffer::create(device, m_perMeshConst,  BufferType::CONSTANTS);

	tsassert(m_perScene);
	tsassert(m_perMesh);

	preloadShaders();
	loadTargets();
}

///////////////////////////////////////////////////////////////////////////////

void ForwardRenderer::preloadShaders()
{
	RenderDevice* device = m_gfx->device();
	
	Path standard(m_gfx->getRootPath());
	standard.addDirectories("shaderbin/Standard.shader");

	tsassert(m_shader.load(device, standard.str()));
}

void ForwardRenderer::loadTargets()
{
	GraphicsDisplayOptions opt;
	m_gfx->getDisplayOptions(opt);

	auto device = m_gfx->device();

	ImageResourceInfo depthInfo;
	depthInfo.width = opt.width;
	depthInfo.height = opt.height;
	depthInfo.type = ImageType::_2D;
	depthInfo.usage = ImageUsage::DSV;
	depthInfo.format = ImageFormat::DEPTH32;
	depthInfo.msLevels = opt.multisampleLevel;
	m_depthBuffer = device->createResourceImage(nullptr, depthInfo);

	ImageView depthView;
	depthView.image = m_depthBuffer.handle();
	depthView.count = 1;
	depthView.index = 0;
	depthView.type = ImageType::_2D;

	ImageView displayView;
	displayView.image = device->getDisplayTarget();
	displayView.count = 1;
	displayView.index = 0;
	displayView.type = ImageType::_2D;

	TargetCreateInfo targetInfo;
	targetInfo.viewport.h = opt.height;
	targetInfo.viewport.w = opt.width;
	targetInfo.scissor = targetInfo.viewport;
	targetInfo.attachments = &displayView;
	targetInfo.attachmentCount = 1;
	targetInfo.depth = depthView;

	m_target = device->createTarget(targetInfo);
	tsassert(m_target);
}

///////////////////////////////////////////////////////////////////////////////

void ForwardRenderer::setCameraView(const Matrix& view)
{
	m_perSceneConst.view = view;
	m_perSceneConst.viewPos = view.inverse().getTranslation();
}

void ForwardRenderer::setCameraProjection(const Matrix& proj)
{
	m_perSceneConst.projection = proj;
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

	//Construct command
	DrawCommandInfo cmd;
	cmd.inputs = item.resources.handle();
	cmd.pipeline = item.pso.handle();
	cmd.outputs = m_target.handle();

	//Command arguments
	if (mesh.data.mode == DrawMode::VERTEX || mesh.data.mode == DrawMode::INSTANCED)
	{
		cmd.start = mesh.data.vertexStart;
		cmd.count = mesh.data.vertexCount;
		cmd.instances = 1;
		cmd.vbase = mesh.data.vertexBase;
	}
	else //indexed
	{
		cmd.start = mesh.data.indexStart;
		cmd.count = mesh.data.indexCount;
		cmd.instances = 1;
		cmd.vbase = mesh.data.vertexBase;
	}

	cmd.mode = mesh.data.mode;

	item.draw = device->createCommand(cmd);

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
	
	ctx->submit(item.draw.handle());
}


void ForwardRenderer::begin()
{
	tsassert(m_gfx);
	auto ctx = m_gfx->device()->context();

	auto constants(m_perSceneConst);
	//Ambient light
	constants.ambient = Vector(0.16f, 0.16f, 0.16f);

	//Directional light
	constants.direct.colour = RGBA(174, 183, 190);
	constants.direct.dir = Vector(1.0f, -1.0f, -1.0f, 0);
	constants.direct.dir.normalize();
	constants.direct.dir = Matrix::transform4D(constants.direct.dir, constants.view);
	constants.direct.dir.normalize();

	Vector dynamicColours[] =
	{
		colours::Green,
		colours::LightBlue,
		colours::Gold,
		colours::Violet
	};

	Vector dynamicPos[] =
	{
		Vector(+10, 5, +10, 1),
		Vector(+10, 5, -10, 1),
		Vector(-10, 5, +10, 1),
		Vector(-10, 5, -10, 1)
	};

	//Dynamic lighting
	for (int i = 0; i < 4; i++)
	{
		constants.dynamic[i].enabled = 1;
		constants.dynamic[i].attConstant = 1.0f;
		constants.dynamic[i].attLinear = 0.1f;
		constants.dynamic[i].attQuadratic = 0.01f;
		constants.dynamic[i].pos = Matrix::transform4D(dynamicPos[i], constants.view);
		constants.dynamic[i].colour = dynamicColours[i];
	}

	Matrix::transpose(constants.view);
	Matrix::transpose(constants.projection);

	//Update scene constants
	ctx->resourceUpdate(m_perScene.handle(), &constants);

	//Clear backbuffer
	ctx->clearDepthTarget(m_target.handle(), 1.0f);
	ctx->clearColourTarget(m_target.handle(), RGBA(80, 166, 100));
}

void ForwardRenderer::end()
{
	// do nothing
}

///////////////////////////////////////////////////////////////////////////////
