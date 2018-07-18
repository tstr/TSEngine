/*
	Sandbox application source
*/

#include "Sandbox.h"

#include <tscore/debug/log.h>
#include <tscore/strings.h>

#include "3D/MaterialReader.h"

using namespace std;
using namespace ts;

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor/destructor
///////////////////////////////////////////////////////////////////////////////////////////////////////
Sandbox::Sandbox(int argc, char** argv) :
	Application(argc, argv),
	m_render(graphics()),
	m_camera(input())
{
	m_renderables = ComponentMap<RenderComponent>(&m_entityManager);
	m_transforms = ComponentMap<TransformComponent>(&m_entityManager);
}

Sandbox::~Sandbox() {}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Application events
///////////////////////////////////////////////////////////////////////////////////////////////////////
int Sandbox::onInit()
{
	input()->addListener(this);

	SSystemInfo sysInfo;
	Application::getSystemInfo(sysInfo);
	tsinfo("OS:   %", sysInfo.osName);
	tsinfo("User: %", sysInfo.userName);

	m_camera.setPosition(Vector(0, 1.0f, -4.0f));
	m_camera.setSpeed(15.0f);
	m_scale = 0.1f;

	///////////////////////////////////////////////////////////////////

	/*
	String model;
	if (getEnv().getVars()->get("Sandbox.model", model))
	{
		if (int err = loadModel(model))
			return err;
	}
	else
	{
		tserror("Variable \"Sandbox.model\" does not exist");
		return -1;
	}
	//*/

	//////////////////////////////////////////////////////////////////////////////
	// Configure forward renderer settings
	//////////////////////////////////////////////////////////////////////////////

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

	m_render.setAmbientColour(RGBA(38, 38, 38));
	m_render.setDirectionalLightColour(RGBA(174, 183, 190));
	m_render.setDirectionalLightDir(Vector(1.0f, -1.0f, -1.0f, 0));

	//Dynamic lighting
	for (size_t i = 0; i < 4; i++)
	{
		m_render.setLightAttenuation(i, 0.01f, 0.1f, 1.0f);
		m_render.setLightPosition(i, dynamicPos[i]);
		m_render.setLightColour(i, dynamicColours[i]);
		m_render.enableDynamicLight(i);
	}

	//Initialize render target
	m_renderTarget = RenderTargets<>(graphics()->device());
	m_renderTarget.attach(0, graphics()->getDisplayView());
	m_renderTarget.attachDepth(graphics()->getDisplayTargetPool()->newDepthTarget(true));
	m_renderTarget.setViewport(graphics()->getDisplayViewport());

	//////////////////////////////////////////////////////////////////////////////

	Entity sponza;
	Entity cube;

	m_entityManager.create(sponza);
	m_entityManager.create(cube);

	{
		if (int err = loadModel(sponza, m_sponzaModel, "sponza/sponza.model"))
			return err;

		//Set transforms
		m_transforms.setComponent(sponza, Matrix::scale(m_scale));

		//Save entities
		m_entities.push_back(sponza);
	}

	{
		if (int err = loadModel(cube, m_cubeModel, "cube.model"))
			return err;

		//Set transforms
		m_transforms.setComponent(cube, Matrix::translation(Vector(0, 1, 0)));

		m_entities.push_back(cube);
	}

	//////////////////////////////////////////////////////////////////////////////

	return 0;
}

void Sandbox::onExit()
{
	input()->removeListener(this);

	tsinfo("exit");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

int Sandbox::loadModel(Entity entity, Model& model, const String& modelfile)
{
	auto absPath = [this](auto r) -> Path
	{
		Path a(graphics()->getRootPath());
		a.addDirectories(r);
		return a;
	};

	if (!model.load(graphics()->device(), absPath(modelfile).str()))
	{
		tserror("unable to import model \"%\"", modelfile);
		return -1;
	}

	RenderComponent component;
	component.items.reserve(model.meshes().size());

	String matfile(modelfile);
	matfile.replace(matfile.find_last_of('.'), string::npos, ".mat");
	MaterialReader matReader(absPath(matfile));

	for (const auto& mesh : model.meshes())
	{
		MeshInfo meshInfo;
		meshInfo.attributeMap = &model.attributes();
		meshInfo.data = mesh;
		meshInfo.topology = VertexTopology::TRIANGLELIST;

		component.items.push_back(m_render.createRenderable(meshInfo, matReader.find(mesh.name)));
	}
	
	m_renderables.setComponent(entity, move(component));

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//	On Application update
///////////////////////////////////////////////////////////////////////////////////////////////////////
void Sandbox::onUpdate(double deltatime)
{
	//Get display dimensions
	Viewport dviewport(graphics()->getDisplayViewport());

	m_camera.setAspectRatio((float)dviewport.w / dviewport.h);
	m_camera.update(deltatime);

	m_render.setCameraView(m_camera.getViewMatrix());
	m_render.setCameraProjection(m_camera.getProjectionMatrix());

	//Update render target viewport
	m_renderTarget.setViewport(dviewport);

	//Begin rendering
	m_render.begin(m_renderTarget);

	// Submit entities for rendering
	for (Entity e : m_entities)
	{
		if (m_transforms.hasComponent(e))
		{
			const TransformComponent& tcomp(m_transforms.getComponent(e));

			//Draw
			if (m_renderables.hasComponent(e))
			{
				const RenderComponent& rcomp(m_renderables.getComponent(e));

				for (const Renderable& item : rcomp.items)
				{
					m_render.draw(item, tcomp.getMatrix());
				}
			}
		}
	}

	//Finish rendering
	m_render.end();

	//tsprofile("x:% y:% z:%", m_camera.getPosition().x(), m_camera.getPosition().y(), m_camera.getPosition().z());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

void Sandbox::onKeyDown(EKeyCode code)
{
	if (code == eKeyEsc)
	{
		exit(0);
	}
	else if (code == eKeyF1)
	{
		GraphicsDisplayOptions opt;
		graphics()->getDisplayOptions(opt);
		graphics()->setDisplayMode((opt.mode == DisplayMode::BORDERLESS) ? DisplayMode::WINDOWED : DisplayMode::BORDERLESS);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
