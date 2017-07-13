/*
	3D Graphics class source
*/

#include "Graphics3D.h"

using namespace ts;
using namespace std;

enum EConstantIndices
{
	INDEX_SCENE_CONSTANTS = 0,
	INDEX_MESH_CONSTANTS = 1,
	INDEX_MATERIAL_CONSTANTS = 2
};

struct DirectLight
{
	Vector colour;
	Vector dir;
};

struct DynamicLight
{
	Vector colour;
	Vector pos;

	int enabled = 0;

	//Attenuation factors
	float attConstant;
	float attLinear;
	float attQuadratic;
};

struct SceneConstants
{
	Matrix view;
	Matrix projection;
	Vector viewPos;
	Vector ambient;
	//Directional light
	DirectLight direct;
	DynamicLight dynamic[4];
};

struct MeshConstants
{
	Matrix world;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////
// ctor/dtor
///////////////////////////////////////////////////////////////////////////////////////////////////////

Graphics3D::Graphics3D(GraphicsSystem* system, Scene* scene) :
	m_context(system),
	m_scene(scene),
	m_graphicsComponents(scene->getEntities()),
	m_lightSourceComponents(scene->getEntities()),
	m_passColour(&m_context)
{
	m_constMesh = m_context.getPool()->createConstantBuffer(MeshConstants());
	m_constScene = m_context.getPool()->createConstantBuffer(SceneConstants());
}

Graphics3D::~Graphics3D()
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Graphics Component methods
///////////////////////////////////////////////////////////////////////////////////////////////////////

void Graphics3D::createGraphicsComponent(Entity e, MeshId mesh, const SubmeshInfo* info, size_t infoCount)
{
	GraphicsComponent component;

	SMeshInstance meshInst;

	//Load mesh instance
	tsassert(getContext()->getMeshManager()->getMeshInstance(mesh, meshInst) == eMeshStatus_Ok);

	//Foreach submesh create a render item
	for (size_t i = 0; i < infoCount; i++)
	{
		SDrawCommand command;

		/*
			Set states
		*/

		// Set default texture sampler
		STextureSampler sampler;
		info[i].states.getSamplerState(sampler);
		sampler.enabled = true;
		command.textureSamplers[0] = sampler;

		// Set render states
		info[i].states.getStates(command.blendState, command.rasterState, command.depthState);

		// Set shader
		SShaderProgram prog;
		getContext()->getShaderManager()->getProgram(info[i].states.getShader(), prog);
		command.shaderVertex = prog.hVertex;
		command.shaderPixel = prog.hPixel;
		command.shaderGeometry = prog.hGeometry;
		command.shaderDomain = prog.hDomain;
		command.shaderHull = prog.hHull;

		/*
			Set resources
		*/

		// Set material constants
		MemoryBuffer materialConstants = info[i].resources.getConstants();
		command.constantBuffers[INDEX_MATERIAL_CONSTANTS] = getContext()->getPool()->createConstantBuffer(materialConstants.pointer(), materialConstants.size());
		command.constantBuffers[INDEX_SCENE_CONSTANTS] = m_constScene;
		command.constantBuffers[INDEX_MESH_CONSTANTS] = m_constMesh;

		// Set textures
		for (auto it = info[i].resources.beginTextureIterator(); it != info[i].resources.endTextureIterator(); it++)
		{
			uint32 idx = it->first;
			TextureId tex = it->second;

			STextureUnit& unit = command.textureUnits[idx];

			//Load texture properties
			STextureProperties props;
			getContext()->getTextureManager()->getTexProperties(tex, props);

			unit.arrayCount = props.arraySize;
			unit.arrayIndex = 0;
			unit.textureType = props.type;

			//Load texture resource handle
			getContext()->getTextureManager()->getTexHandle(tex, unit.texture);
		}

		// Set index buffer
		command.indexBuffer = meshInst.indexBuffer;

		// Copy vertex information
		copy(begin(meshInst.vertexBuffers), end(meshInst.vertexBuffers), command.vertexBuffers);
		copy(begin(meshInst.vertexOffset), end(meshInst.vertexOffset), command.vertexOffsets);
		copy(begin(meshInst.vertexStrides), end(meshInst.vertexStrides), command.vertexStrides);

		command.vertexCount = 0;
		command.vertexStart = 0;

		// Copy vertex attributes
		for (uint32 i = 0; i < meshInst.vertexAttributeCount; i++)
			command.vertexAttribs[i] = meshInst.vertexAttributes[i];

		command.vertexAttribCount = meshInst.vertexAttributeCount;

		// Set vertex topology
		command.vertexTopology = meshInst.topology;

		//Set command parameters
		command.vertexBase = info[i].submeshView.vertexBase;
		command.indexCount = info[i].submeshView.indexCount;
		command.indexStart = info[i].submeshView.indexOffset;

		// Command mode
		command.mode = EDrawMode::eDrawIndexed;

		/*
			Create Render item
		*/
		Item item = getContext()->createItem();

		m_passColour.add(item, command);

		component.items.push_back(item);
	}

	//Attach component to this entity
	m_graphicsComponents.setComponent(e, component);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Queue methods
///////////////////////////////////////////////////////////////////////////////////////////////////////

//Submit draw command to queue
void Graphics3D::submit(Entity entity)
{
	//todo: check if entity has this component
	if (m_scene->getEntities()->alive(entity))
	{
		GraphicsComponent gcomp;
		m_graphicsComponents.getComponent(entity, gcomp);


		GraphicsDisplayOptions opt;
		getContext()->getSystem()->getDisplayOptions(opt);

		CommandQueue* q = getContext()->getQueue();

		MeshConstants constants;
		constants.world = m_scene->getTransform(entity);
		Matrix::transpose(constants.world);

		//Allocate command batch
		CommandBatch* batch = q->createBatch();
		//Update model constants
		q->addCommand(batch, CommandBufferUpdate(m_constMesh), constants);

		//For each render item in this component
		for (Item i : gcomp.items)
		{
			//Enqueue for colour pass
			q->addCommand(batch, CommandDraw(m_passColour.getTarget(), m_passColour.get(i), SViewport(opt.width, opt.height, 0, 0), SViewport()));
		}

		//Submit command batch
		q->submitBatch(1, batch);
	}
}

void Graphics3D::update()
{
	HTarget target = getContext()->getSystem()->getDisplayTarget();
	CommandQueue* q = getContext()->getQueue();

	//////////////////////////////////////////////////////////////////////////////////////

	SceneConstants constants;
	constants.view = m_scene->getCamera()->getViewMatrix();
	constants.projection = m_scene->getCamera()->getProjectionMatrix();
	constants.viewPos = constants.view.inverse().getTranslation();

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

	Vector v = Vector(1.0f, 0.0, 0.0) * 0.01f;

	Matrix::transpose(constants.view);
	Matrix::transpose(constants.projection);

	//////////////////////////////////////////////////////////////////////////////////////

	CommandBatch* batch = q->createBatch();
	//q->addCommand(batch, CommandTargetClear(target, (const Vector&)colours::Red, 1.0f));
	q->addCommand(batch, CommandBufferUpdate(m_constScene), constants);
	q->submitBatch(0, batch);

	getContext()->commit();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
