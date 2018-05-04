/*
	Sandbox application source
*/

#include "Sandbox.h"

#include <tscore/debug/log.h>

using namespace std;
using namespace ts;

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor/destructor
///////////////////////////////////////////////////////////////////////////////////////////////////////
Sandbox::Sandbox(EngineEnv& env) :
	Application(env),
	m_g3D(getEnv().getGraphics(), &m_scene),
	m_scene(&m_entityManager, getEnv().getInput())
{}

Sandbox::~Sandbox() {}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Application events
///////////////////////////////////////////////////////////////////////////////////////////////////////
int Sandbox::onInit()
{
	getEnv().getInput()->addListener(this);

	SSystemInfo sysInfo;
	EngineEnv::getSystemInfo(sysInfo);
	tsinfo("OS:   %", sysInfo.osName);
	tsinfo("User: %", sysInfo.userName);

	//Print variables
	VarTable::List varList = getEnv().getVars()->toList();
	for (auto entry : varList)
	{
		tsinfo("% = %", entry.name, entry.value);
	}

	getScene()->getCamera()->setPosition(Vector(0, 1.0f, -4.0f));
	getScene()->getCamera()->setSpeed(15.0f);
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
		if (int err = loadModel(sponza, "sponza/sponza.model"))
			return err;

		//Set transforms
		getScene()->setTransform(sponza, Matrix::scale(m_scale));

		//Save entities
		m_entities.push_back(sponza);
	}

	{
		if (int err = loadModel(cube, "cube.model"))
			return err;

		//Set transforms
		getScene()->setTransform(cube, Matrix::translation(Vector(0, 1, 0)));

		m_entities.push_back(cube);
	}

	///////////////////////////////////////////////////////////////////

	return 0;
}

void Sandbox::onExit()
{
	getEnv().getInput()->removeListener(this);

	tsinfo("exit");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

int Sandbox::loadModel(Entity entity, const String& modelfile)
{
	Path p(m_g3D.getContext()->getSystem()->getRootPath());
	p.addDirectories(modelfile);
	CModel model(m_g3D.getContext());

	if (!model.import(p))
	{
		tserror("unable to import model \"%\"", p.str());
		return -1;
	}

	std::vector<SubmeshInfo> submeshes;

	for (auto it = model.beginSection(); it != model.endSection(); it++)
	{
		const SMaterial& material = it->material;

		SubmeshInfo info;

		ShaderId program;
		if (auto status = m_g3D.getContext()->getShaderManager()->load("Standard", program))
		{
			return status;
		}

		//Shader program
		info.states.setShader(program);

		//Render states
		info.states.setCullMode(eCullBack);
		info.states.setFillMode(eFillSolid);
		info.states.enableAlpha(false);
		info.states.enableDepth(true);
		info.states.setSamplerAddressMode(eTextureAddressWrap);
		info.states.setSamplerFiltering(eTextureFilterAnisotropic16x);

		//Shader constants
		info.resources.setConstants(material.params);

		//Textures
		info.resources.setTexture(0, material.diffuseMap);
		info.resources.setTexture(1, material.normalMap);

		info.submeshView = it->submesh;

		submeshes.push_back(info);
	}

	//Attach a graphics component to entity
	m_g3D.createGraphicsComponent(entity, model.getMeshID(), &submeshes[0], submeshes.size());

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//	On Application update
///////////////////////////////////////////////////////////////////////////////////////////////////////
void Sandbox::onUpdate(double deltatime)
{
	GraphicsDisplayOptions displayOpt;
	getEnv().getGraphics()->getDisplayOptions(displayOpt);
	
	getScene()->getCamera()->setAspectRatio((float)displayOpt.width / displayOpt.height);
	getScene()->getCamera()->update(deltatime);

	// Submit entities for rendering
	for (Entity e : m_entities)
	{
		m_g3D.submit(e);
	}

	//tsprofile("x:% y:% z:%", m_camera.getPosition().x(), m_camera.getPosition().y(), m_camera.getPosition().z());

	m_g3D.update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

void Sandbox::onKeyDown(EKeyCode code)
{
	if (code == eKeyEsc)
	{
		getEnv().exit(0);
	}
	else if (code == eKeyF1)
	{
		GraphicsDisplayOptions opt;
		getEnv().getGraphics()->getDisplayOptions(opt);
		getEnv().getGraphics()->setDisplayMode((opt.mode == eDisplayBorderless) ? eDisplayWindowed : eDisplayBorderless);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
