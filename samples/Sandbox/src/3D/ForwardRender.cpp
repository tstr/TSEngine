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

	m_shadowMap = ShadowMap(m_gfx, 1024, 1024);

	preloadShaders();
}

///////////////////////////////////////////////////////////////////////////////

void ForwardRenderer::preloadShaders()
{
	RenderDevice* device = m_gfx->device();

	auto load = [this, device](ShaderProgram& program, const char* path) {
		Path standard(m_gfx->getRootPath());
		standard.addDirectories(path);
		tsassert(program.load(device, standard.str()));
	};

	load(m_shader,        "shaderbin/Standard.shader");
	load(m_shaderNormMap, "shaderbin/StandardNormalMapped.shader");
}

ShaderHandle ForwardRenderer::selectShader(const Mesh& mesh, const PhongMaterial& mat)
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

///////////////////////////////////////////////////////////////////////////////

Renderable ForwardRenderer::createRenderable(const Mesh& mesh, const PhongMaterial& phong)
{
	tsassert(m_gfx);

	RenderDevice* device = m_gfx->device();

	Renderable item;

	/*
		Setup material parameters
	*/

	//Set image binding if the given view is not null
	auto bindIf = [&](ImageBindings binding, const ImageView& view)
	{
		if (view.image != ResourceHandle())
			item.mat.images[binding] = view;
	};

	//Material shader resources
	bindIf(BIND_DIFFUSE_MAP, phong.diffuseMap);
	bindIf(BIND_NORMAL_MAP, phong.normalMap);
	bindIf(BIND_SPECULAR_MAP, phong.specularMap);
	bindIf(BIND_DISPLACEMENT_MAP, phong.displacementMap);

	//Shadow map
	item.mat.images[BIND_SHADOW_MAP] = m_shadowMap.getView();

	//Material shader constants
	MaterialConstants matConstants;
	matConstants.ambientColour = phong.ambientColour;
	matConstants.diffuseColour = phong.diffuseColour;
	matConstants.emissiveColour = phong.emissiveColour;
	matConstants.specularColour = phong.specularColour;
	matConstants.specularPower = phong.specularPower;

	item.mat.buffer = Buffer::create(device, matConstants, BufferType::CONSTANTS);

	/*
		Renderable item parameters
	*/
	item.inputs = makeResourceSet(mesh, item.mat);
	item.pso = makePipeline(mesh, phong);
	item.params = mesh.getParams();

	makeShadowPipelineAndResources(item, mesh);

	return move(item);
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
	info.vertexBuffers = &mesh.getBufferView();
	info.vertexBufferCount = 1;
	info.indexBuffer = mesh.indices;

	return device->createResourceSet(info);
}

void findAttribute(const char* semantic, VertexAttributeType type, const VertexAttributeMap& attribMap, vector<VertexAttribute>& attribs)
{
	auto it = attribMap.find(semantic);
	if (it != attribMap.end())
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

RPtr<PipelineHandle> ForwardRenderer::makePipeline(const Mesh& mesh, const PhongMaterial& mat)
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

	findAttribute("POSITION",  VertexAttributeType::FLOAT4, mesh.vertexAttributes, attrib);
	findAttribute("TEXCOORD0", VertexAttributeType::FLOAT2, mesh.vertexAttributes, attrib);
	findAttribute("COLOUR0",   VertexAttributeType::FLOAT4, mesh.vertexAttributes, attrib);
	findAttribute("NORMAL",    VertexAttributeType::FLOAT3, mesh.vertexAttributes, attrib);
	findAttribute("TANGENT",   VertexAttributeType::FLOAT3, mesh.vertexAttributes, attrib);
	findAttribute("BITANGENT", VertexAttributeType::FLOAT3, mesh.vertexAttributes, attrib);

	pso.vertexAttributeCount = attrib.size();
	pso.vertexAttributeList = attrib.data();
	
	//Vertex topology
	pso.topology = mesh.vertexTopology;

	return device->createPipeline(selectShader(mesh, mat), pso);
}

///////////////////////////////////////////////////////////////////////////////
//	Draw submission
///////////////////////////////////////////////////////////////////////////////

void ForwardRenderer::draw(const Renderable& item, const Matrix& transform)
{
	m_renderQueue.emplace_back(&item, transform);
}

void ForwardRenderer::begin(ForwardRenderTarget& targets)
{
	m_targets = targets.handle();

	tsassert(m_gfx);
	tsassert(m_targets != TargetHandle());

	auto ctx = m_gfx->device()->context();

	//Clear backbuffer
	ctx->clearDepthTarget(m_targets, 1.0f);
	ctx->clearColourTarget(m_targets, RGBA(80, 166, 100));

	//Clear shadow buffer
	ctx->clearDepthTarget(m_shadowMap.getTarget(), 1.0f);
	ctx->clearColourTarget(m_shadowMap.getTarget(), RGBA(255, 255, 255));
}

void ForwardRenderer::end()
{
	auto ctx = m_gfx->device()->context();

	SceneConstants constants;

	//////////////////////////////////////////////////////////////////////////
	//	Shadow pass
	//////////////////////////////////////////////////////////////////////////

	float zFar = 150.0f;
	float zNear = 0.1f;

	Matrix lightView, lightProjection;

	//Converts world space => light space
	//lightView = Matrix(Vector(0, 0, 0), m_directLightDir, Vector(0, 1, 0));
	//lightProjection = Matrix::orthographic(100, 100, 0.1f, 100.0f);

	//lightView = Matrix::translation(10, 10, 0).inverse();
	lightView = Matrix::lookTo(Vector(20, 5, 0), Vector(1, 0, -0.5), Vector(0, 1, 0));
	//lightView = Matrix::lookTo(Vector(20, 25, 0), Vector(1, -0.5, -0.5), Vector(0, 1, 0));
	//lightView = Matrix::lookAt(Vector(0, 0, 0), m_directLightDir * zFar, Vector(0, 1, 0));

	lightProjection = Matrix::perspectiveFieldOfView(Pi / 2, 1, zNear, zFar);
	//lightProjection = Matrix::orthographic(zFar, zFar, zNear, zFar);
	
	constants.view = lightView;
	constants.projection = lightProjection;
	Matrix::transpose(constants.view);
	Matrix::transpose(constants.projection);

	//Update scene constants
	ctx->resourceUpdate(m_perScene.handle(), &constants);

	bool debugShadowMap = false;

	//*
	//shadow pass
	for (const auto& r : m_renderQueue)
	{
		MeshConstants constants;
		constants.world = r.transform.transpose();
		ctx->resourceUpdate(m_perMesh.handle(), &constants);

		ctx->draw(
			debugShadowMap ? m_targets : m_shadowMap.getTarget(),
			r.item->shadowPso.handle(),
			r.item->shadowInputs.handle(),
			r.item->params
		);
	}
	//*/

	//////////////////////////////////////////////////////////////////////////
	//	colour pass
	//////////////////////////////////////////////////////////////////////////

	//Ambient light
	constants.ambient = m_ambientColour;
	constants.view = m_viewMatrix;
	constants.projection = m_projMatrix;
	constants.lightView = constants.view.inverse() * lightView * lightProjection;

	Vector vec(0, 0, 0, 1);
	vec = Matrix::transform4D(vec, constants.view);
	vec = Matrix::transform4D(vec, constants.lightView);

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
	Matrix::transpose(constants.lightView);

	//Update scene constants
	ctx->resourceUpdate(m_perScene.handle(), &constants);

	if (!debugShadowMap)
	for (const auto& r : m_renderQueue)
	{
		//Update mesh constants
		MeshConstants constants;
		constants.world = r.transform.transpose();

		ctx->resourceUpdate(m_perMesh.handle(), &constants);

		ctx->draw(
			m_targets,
			r.item->pso.handle(),
			r.item->inputs.handle(),
			r.item->params
		);
	}

	//////////////////////////////////////////////////////////////////////////

	m_renderQueue.clear();
}

///////////////////////////////////////////////////////////////////////////////

void ForwardRenderer::makeShadowPipelineAndResources(Renderable& item, const Mesh& mesh)
{
	auto device = m_gfx->device();

	ResourceSetCreateInfo info;

	//Constant buffer resources
	BindingSet<ResourceHandle> constantBuffers;
	constantBuffers[BIND_SCENE_CONSTANTS] = m_perScene.handle();
	constantBuffers[BIND_MESH_CONSTANTS] = m_perMesh.handle();

	info.constantBuffers = constantBuffers.data();
	info.constantBuffersCount = (uint32)constantBuffers.count();

	//Mesh resources
	info.vertexBuffers = &mesh.getBufferView();
	info.vertexBufferCount = 1;
	info.indexBuffer = mesh.indices;

	item.shadowInputs = device->createResourceSet(info);


	PipelineCreateInfo pso;
	//States
	pso.blend.enable = false;
	pso.depth.enableDepth = true;
	pso.depth.enableStencil = false;
	pso.raster.enableScissor = false;
	pso.raster.cullMode = CullMode::BACK;
	pso.raster.fillMode = FillMode::SOLID;

	//Vertex layout
	vector<VertexAttribute> attrib;

	findAttribute("POSITION",  VertexAttributeType::FLOAT4, mesh.vertexAttributes, attrib);
	findAttribute("TEXCOORD0", VertexAttributeType::FLOAT2, mesh.vertexAttributes, attrib);
	findAttribute("COLOUR0",   VertexAttributeType::FLOAT4, mesh.vertexAttributes, attrib);
	findAttribute("NORMAL",    VertexAttributeType::FLOAT3, mesh.vertexAttributes, attrib);
	findAttribute("TANGENT",   VertexAttributeType::FLOAT3, mesh.vertexAttributes, attrib);
	findAttribute("BITANGENT", VertexAttributeType::FLOAT3, mesh.vertexAttributes, attrib);

	pso.vertexAttributeCount = attrib.size();
	pso.vertexAttributeList = attrib.data();

	//Vertex topology
	pso.topology = mesh.vertexTopology;

	item.shadowPso = device->createPipeline(m_shadowMap.getShader(), pso);
}

///////////////////////////////////////////////////////////////////////////////