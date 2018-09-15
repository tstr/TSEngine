/*
	3D Scene Graphics class
*/

#include <tscore/debug/assert.h>

#include "SceneRender.h"

using namespace ts;

///////////////////////////////////////////////////////////////////////////////

SceneRender::SceneRender(
	GraphicsSystem* graphics
) : m_gfx(graphics),
	m_materialManager(graphics)
{
	tsassert(m_gfx);
	RenderDevice* device = m_gfx->device();

	m_perScene = Buffer::create(device, SceneConstants(), BufferType::CONSTANTS);
	m_perMesh  = Buffer::create(device, MeshConstants(), BufferType::CONSTANTS);

	tsassert(m_perScene);
	tsassert(m_perMesh);

	m_targets = RenderTargets<>(device);
	m_targets.attach(0, m_gfx->getDisplayView());
	m_targets.attachDepth(m_gfx->getDisplayTargetPool()->newDepthTarget(true));
	m_targets.setViewport(m_gfx->getDisplayViewport());

	m_shadowPass = ShadowPass(device, 2048, 2048);
}

///////////////////////////////////////////////////////////////////////////////

void SceneRender::update()
{
	tsassert(m_gfx);

	//Get display dimensions
	Viewport dviewport(m_gfx->getDisplayViewport());
	m_targets.setViewport(dviewport);
	
	tsassert(m_targets.handle() != TargetHandle());

	auto ctx = m_gfx->device()->context();

	//Clear backbuffer
	ctx->clearDepthTarget(m_targets.handle(), 1.0f);
	ctx->clearColourTarget(m_targets.handle(), RGBA(80, 166, 100));

	//Clear shadow buffer
	ctx->clearDepthTarget(m_shadowPass.getTarget(), 1.0f);
	ctx->clearColourTarget(m_shadowPass.getTarget(), RGBA(255, 255, 255));

	//Shadow pass
	executeShadowPass(
		m_visibleRenderables
	);

	//Colour pass
	executeColourPass(
		m_targets.handle(),
		m_visibleRenderables
	);

	m_visibleRenderables.clear();
}


void SceneRender::executeColourPass(TargetHandle target, const RenderableList& renderables)
{
	auto ctx = m_gfx->device()->context();

	SceneConstants constants;

	//Ambient light
	constants.ambient = m_ambientColour;
	constants.view = m_viewMatrix;
	constants.projection = m_projMatrix;
	constants.lightView = m_lightView * m_lightProjection;

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

	for (const auto& r : m_visibleRenderables)
	{
		//Update mesh constants
		MeshConstants constants;
		constants.world = r.transform.transpose();

		ctx->resourceUpdate(m_perMesh.handle(), &constants);

		ctx->draw(
			target,
			r.item->pso.handle(),
			r.item->inputs.handle(),
			r.item->params
		);
	}
}

void SceneRender::executeShadowPass(const RenderableList& renderables)
{
	auto ctx = m_gfx->device()->context();

	SceneConstants constants;

	const float zFar = 150.0f;
	const float zNear = 0.001f;

	//todo: calculate from view frustum

	//Converts world space => light space
	m_lightView = Matrix::lookAt(Vector(0, 140, 0), Vector(), Vector(1, 0, 0));
	//lightProjection = Matrix::perspectiveFieldOfView(Pi / 2, 1, zNear, zFar);
	m_lightProjection = Matrix::orthographic(400, 400, zNear, zFar);

	constants.view = m_lightView;
	constants.projection = m_lightProjection;
	Matrix::transpose(constants.view);
	Matrix::transpose(constants.projection);

	//Update scene constants
	ctx->resourceUpdate(m_perScene.handle(), &constants);

	//shadow pass
	for (const auto& r : m_visibleRenderables)
	{
		MeshConstants constants;
		constants.world = r.transform.transpose();
		ctx->resourceUpdate(m_perMesh.handle(), &constants);

		ctx->draw(
			m_shadowPass.getTarget(),
			r.item->shadowPso.handle(),
			r.item->shadowInputs.handle(),
			r.item->params
		);
	}
}

///////////////////////////////////////////////////////////////////////////////

Renderable SceneRender::createRenderable(const Mesh& mesh, const PhongMaterial& phong)
{
	tsassert(m_gfx);

	RenderDevice* device = m_gfx->device();

	Renderable item;

	/*
		Setup material parameters
	*/

	BindingSet<ImageView> images;

	//Set image binding if the given view is not null
	auto bindIf = [&](ImageBindings binding, const ImageView& view)
	{
		if (view.image != ResourceHandle())
			images[binding] = view;
	};

	//Material shader resources
	bindIf(BIND_DIFFUSE_MAP, phong.diffuseMap);
	bindIf(BIND_NORMAL_MAP, phong.normalMap);
	bindIf(BIND_SPECULAR_MAP, phong.specularMap);
	bindIf(BIND_DISPLACEMENT_MAP, phong.displacementMap);

	//Shadow map
	images[BIND_SHADOW_MAP] = m_shadowPass.getView();

	//Material shader constants
	MaterialConstants matConstants;
	matConstants.ambientColour = phong.ambientColour;
	matConstants.diffuseColour = phong.diffuseColour;
	matConstants.emissiveColour = phong.emissiveColour;
	matConstants.specularColour = phong.specularColour;
	matConstants.specularPower = phong.specularPower;

	item.materialBuffer = Buffer::create(device, matConstants, BufferType::CONSTANTS);

	/*
		Resource Set
	*/
	ResourceSetCreateInfo info;

	//Constant buffer resources
	BindingSet<ResourceHandle> constantBuffers;
	constantBuffers[BIND_SCENE_CONSTANTS] = m_perScene.handle();
	constantBuffers[BIND_MESH_CONSTANTS] = m_perMesh.handle();
	constantBuffers[BIND_MAT_CONSTANTS] = item.materialBuffer.handle();

	info.constantBuffers = constantBuffers.data();
	info.constantBuffersCount = (uint32)constantBuffers.count();

	//Image resources
	info.resources = images.data();
	info.resourceCount = (uint32)images.count();

	//Mesh resources
	info.vertexBuffers = &mesh.getBufferView();
	info.vertexBufferCount = 1;
	info.indexBuffer = mesh.indices;

	//Colour pass resources
	item.inputs = device->createResourceSet(info);
	item.pso = m_materialManager.getForwardPipeline(mesh, phong);

	//Unbind shadow map for now
	images[BIND_SHADOW_MAP] = ImageView();

	//Shadow pass
	item.shadowInputs = device->createResourceSet(info);
	item.shadowPso = m_materialManager.getShadowPipeline(mesh, phong);

	/*
		Mesh
	*/
	item.params = mesh.getParams();

	return std::move(item);
}

///////////////////////////////////////////////////////////////////////////////
