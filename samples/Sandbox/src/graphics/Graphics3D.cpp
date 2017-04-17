/*
	3D Graphics class source
*/

#include "Graphics3D.h"
#include "Model.h"

using namespace ts;
using namespace std;

struct SceneParams
{
	Matrix view;
	Matrix projection;
	Vector viewPos;
};

struct MeshParams
{
	Matrix world;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////
// ctor/dtor
///////////////////////////////////////////////////////////////////////////////////////////////////////

Graphics3D::Graphics3D(GraphicsSystem* system) :
	GraphicsContext(system)
{
	auto gfx = GraphicsContext::getSystem();

	m_sceneBuffer = getPool()->createConstantBuffer(SceneParams());
}

Graphics3D::~Graphics3D()
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Queue methods
///////////////////////////////////////////////////////////////////////////////////////////////////////

//Submit draw command to queue
void Graphics3D::draw(const CRenderItem& item)
{
	SGraphicsDisplayInfo info;
	getSystem()->getDisplayInfo(info);
	HTarget target = getSystem()->getDisplayTarget();

	CommandQueue* q = GraphicsContext::getQueue();

	CommandBatch* batch = q->createBatch();
	q->addCommand(batch, CommandDraw(target, item.getCommand(), SViewport(info.width, info.height, 0, 0), SViewport()));
	q->submitBatch(1, batch);
}

void Graphics3D::update()
{
	HTarget target = getSystem()->getDisplayTarget();
	CommandQueue* q = GraphicsContext::getQueue();

	SceneParams sceneParam;
	sceneParam.view = m_matrixView;
	sceneParam.projection = m_matrixProj;
	sceneParam.viewPos = sceneParam.view.inverse().getTranslation();
	Matrix::transpose(sceneParam.view);
	Matrix::transpose(sceneParam.projection);

	CommandBatch* batch = q->createBatch();
	//q->addCommand(batch, CommandTargetClear(target, (const Vector&)colours::Red, 1.0f));
	q->addCommand(batch, CommandBufferUpdate(m_sceneBuffer), sceneParam);
	q->submitBatch(0, batch);

	GraphicsContext::commit();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
