/*
	Common Command Dispatchers used as input for the CommandQueue class
*/

#include <tsgraphics/CommandQueue.h>
#include <tsgraphics/api/RenderApi.h>

using namespace std;
using namespace ts;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Command dispatcher implementations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CommandDraw::dispatch(IRenderContext* context, CommandPtr data)
{
	context->draw(drawTarget, drawView, drawScissor, drawCmd);
}

void CommandBufferUpdate::dispatch(IRenderContext* context, CommandPtr data)
{
	context->bufferUpdate(this->hBuf, data);
}

void CommandTextureUpdate::dispatch(IRenderContext* context, CommandPtr data)
{
	context->textureUpdate(this->hTex, this->texIdx, data);
}

void CommandTextureResolve::dispatch(IRenderContext* context, CommandPtr extra)
{
	context->textureResolve(hSrc, hDst);
}

void CommandTargetClear::dispatch(IRenderContext* context, CommandPtr extra)
{
	context->clearRenderTarget(hTarget, colour);
	context->clearDepthTarget(hTarget, depth);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
