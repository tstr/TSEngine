/*
	Common Command Dispatchers used as input for the CommandQueue class
*/

#include <tsgraphics/CommandQueue.h>
#include <tsgraphics/Driver.h>

using namespace std;
using namespace ts;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Command dispatcher implementations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandDraw::dispatch(RenderContext* context, CommandPtr data)
{
	context->draw(outputs, pipeline, inputs, params);
}

void CommandBufferUpdate::dispatch(RenderContext* context, CommandPtr data)
{
	context->resourceUpdate(this->hBuf, data);
}

void CommandTextureUpdate::dispatch(RenderContext* context, CommandPtr data)
{
	context->resourceUpdate(this->hImg, data, this->index);
}

void CommandTextureResolve::dispatch(RenderContext* context, CommandPtr extra)
{
	context->imageResolve(hSrc, hDst);
}

void CommandTargetClear::dispatch(RenderContext* context, CommandPtr extra)
{
	context->clearColourTarget(hTarget, RGBA(colour));
	context->clearDepthTarget(hTarget, depth);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
