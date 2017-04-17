/*
	Sandbox application source
*/

#include "Sandbox.h"
#include "util/info.h"

#include <tscore/debug/log.h>

#include "graphics/Model.h"

using namespace std;
using namespace ts;

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor/destructor
///////////////////////////////////////////////////////////////////////////////////////////////////////
Sandbox::Sandbox(CEngineEnv& env) :
	mEnv(env),
	m_g3D(env.getGraphics()),
	m_camera(env.getInput())
{}

Sandbox::~Sandbox() {}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Application events
///////////////////////////////////////////////////////////////////////////////////////////////////////
int Sandbox::onInit()
{
	//Print information
	printRepositoryInfo();
	printSystemInfo();

	m_camera.setPosition(Vector(0, 1.0f, -4.0f));
	m_camera.setSpeed(6.0f);

	////////////////////////////////////////////////////////////////////////////////////
	//*

	const char modelfile[] = "cube.tsm";
	//const char modelfile[] = "sponza/sponza.tsm";

	Path p(m_g3D.getSystem()->getRootPath());
	p.addDirectories(modelfile);
	CModel model(m_g3D.createModel());

	if (!model.import(p, eModelVertexAttributePosition | eModelVertexAttributeTexcoord))
	{
		tswarn("unable to import model \"%\"", p.str());
		return -1;
	}

	for (uint32 i = 0; i < model.getMeshCount(); i++)
	{
		SMesh mesh = model.getMesh(i);

		CRenderItemInfo info(m_g3D.createInfo());
		m_g3D.setScene(info);

		SDepthState depthState;
		SRasterState rasterState;
		SBlendState blendState;

		depthState.enableDepth = true;
		blendState.enable = false;
		rasterState.enableScissor = false;
		rasterState.cullMode = eCullBack;
		rasterState.fillMode = eFillSolid;

		info.setRasterState(rasterState);
		info.setBlendState(blendState);
		info.setDepthState(depthState);

		info.setMesh(mesh.id);
		info.setTexture(0, mesh.material.diffuseMap);
		info.setDrawIndexed(mesh.vertexBase, mesh.indexOffset, mesh.indexCount);

		CRenderItem item(m_g3D.createItem(info));

		if (item.getCommand() == HDRAWCMD_NULL)
		{
			tserror("Error creating draw command");
		}

		m_commands.push_back(move(item));
	}

	//*/
	////////////////////////////////////////////////////////////////////////////////////

	return 0;
}

void Sandbox::onExit()
{
	tsinfo("exit");
}

void Sandbox::onUpdate(double deltatime)
{
	SGraphicsDisplayInfo displayInf;
	mEnv.getGraphics()->getDisplayInfo(displayInf);

	m_camera.setAspectRatio((float)displayInf.width / displayInf.height);
	m_camera.update(deltatime);

	m_g3D.setView(m_camera.getViewMatrix());
	m_g3D.setProjection(m_camera.getProjectionMatrix());

	for (const CRenderItem& item : m_commands)
	{
		m_g3D.draw(item);
	}

	//tsprofile("x:% y:% z:%", m_camera.getPosition().x(), m_camera.getPosition().y(), m_camera.getPosition().z());

	m_g3D.update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
