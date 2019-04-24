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

	m_render.setAmbientColour(RGBA(30, 30, 30));
	m_render.setDirectionalLightColour(RGBA(205, 215, 225));
	m_render.setDirectionalLightDir(Vector(1.0f, -1.0f, -1.0f, 0));

	//Dynamic lighting
	for (size_t i = 0; i < 4; i++)
	{
		m_render.setLightAttenuation((LightSource)i, 0.01f, 0.1f, 1.0f);
		m_render.setLightPosition((LightSource)i, dynamicPos[i]);
		m_render.setLightColour((LightSource)i, dynamicColours[i]);
		m_render.enableDynamicLight((LightSource)i);
	}

	//////////////////////////////////////////////////////////////////////////////

	m_modelEntity = m_entityManager.create();
	m_boxEntity   = m_entityManager.create();

	{
		if (!addModelRenderComponent(m_modelEntity, "sponza/sponza.model"))
			return -1;

		//Set transforms
		m_transforms.setComponent(m_modelEntity, Matrix::scale(m_scale));
	}

	{
		if (!addModelRenderComponent(m_boxEntity, "cube.model"))
			return -1;

		//Set transforms
		m_transforms.setComponent(m_boxEntity, Matrix::translation(Vector(0, 1, 0)));
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

bool Sandbox::addModelRenderComponent(Entity entity, const String& modelfile)
{
	const Model& model = graphics()->getModel(modelfile);

	if (model.error())
	{
		tserror("unable to import model \"%\"", modelfile);
		return false;
	}

	RenderComponent component;
	component.items.reserve(model.meshes().size());

	MaterialReader matReader(graphics(), model.materialFile());

	for (const auto& mesh : model.meshes())
	{
		PhongMaterial mat(matReader.find(mesh.name));
		mat.enableAlpha = false;
		component.items.push_back(m_render.createRenderable(mesh, mat));
	}
	
	m_renderables.setComponent(entity, move(component));

	return true;
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

	// Submit entities for rendering
	for (Entity e : { m_modelEntity, m_boxEntity })
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

	//Commit renderables
	m_render.update();

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
