/*
	Sandbox application source
*/

#include "Sandbox.h"

#include <tscore/debug/log.h>
#include <tscore/strings.h>

#include "util/IniReader.h"

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

	///////////////////////////////////////////////////////////////////

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

	unordered_map<String, MaterialCreateInfo> materialInfoMap;
	RenderComponent component;
	component.items.reserve(model.meshes().size());

	//////////////////////////////////////////////////////////////////////////////

	String matfile(modelfile);
	matfile.replace(matfile.find_last_of('.'), string::npos, ".mat");
	INIReader matReader(absPath(matfile));

	vector<String> materialSections;
	vector<String> materialProperies;
	String propKey;
	matReader.getSections(materialSections);

	//Helper function
	auto getVectorProperty = [](const String& value) -> Vector
	{
		if (trim(value) == "")
			return Vector();

		Vector v;
		vector<string> tokens = split(value, ',');

		if (tokens.size() > 0) v.x() = stof(tokens[0]);
		if (tokens.size() > 1) v.y() = stof(tokens[1]);
		if (tokens.size() > 2) v.z() = stof(tokens[2]);
		if (tokens.size() > 3) v.w() = stof(tokens[3]);

		return v;
	};

	for (const auto& section : materialSections)
	{
		MaterialCreateInfo matInfo;

		//Format key helper
		auto fmtKey = [&section](auto key)
		{
			return (String)section + "." + key;
		};

		float alpha;
		float shininess;
		matReader.getProperty(fmtKey("alpha"), alpha);
		matReader.getProperty(fmtKey("shininess"), shininess);

		if (matReader.getProperty(fmtKey("diffuseColour"), propKey))
			matInfo.constants.diffuseColour = getVectorProperty(propKey);
		propKey.clear();
		if (matReader.getProperty(fmtKey("ambientColour"), propKey))
			matInfo.constants.ambientColour = getVectorProperty(propKey);
		propKey.clear();
		if (matReader.getProperty(fmtKey("emissiveColour"), propKey))
			matInfo.constants.emissiveColour = getVectorProperty(propKey);
		propKey.clear();

		for (const char* imageKey : {
			"diffuseMap",
			"normalMap",
			"specularMap",
			"displacementMap"
		})
		{
			propKey.clear();

			if (matReader.getProperty(fmtKey(imageKey), propKey))
			{
				if (propKey != "")
				{
					//Resolve image path
					Path a(matReader.getPath().getParent());
					a.addDirectories(propKey);

					matInfo.images[imageKey] = a;
				}
			}
		}

		String name(section);
		toLower(name);
		materialInfoMap[name] = matInfo;
	}

	//////////////////////////////////////////////////////////////////////////////

	for (const auto& mesh : model.meshes())
	{
		MeshInfo meshInfo;
		meshInfo.attributeMap = &model.attributes();
		meshInfo.data = mesh;
		meshInfo.topology = VertexTopology::TRIANGLELIST;

		String name(mesh.name);
		ts::toLower(name);
		auto it = materialInfoMap.find(name);
		MaterialCreateInfo matInfo = (it != materialInfoMap.end()) ? it->second : MaterialCreateInfo();

		component.items.push_back(m_render.createRenderable(meshInfo, matInfo));
	}
	
	m_renderables.setComponent(entity, move(component));

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//	On Application update
///////////////////////////////////////////////////////////////////////////////////////////////////////
void Sandbox::onUpdate(double deltatime)
{
	GraphicsDisplayOptions displayOpt;
	graphics()->getDisplayOptions(displayOpt);
	
	m_camera.setAspectRatio((float)displayOpt.width / displayOpt.height);
	m_camera.update(deltatime);

	m_render.setCameraView(m_camera.getViewMatrix());
	m_render.setCameraProjection(m_camera.getProjectionMatrix());

	m_render.begin();

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
