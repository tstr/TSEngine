/*
	Render API

	D3D11Render target handling methods
*/

#include "Render.h"
#include "HandleTarget.h"

using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RPtr<TargetHandle> Dx11::createTarget(const TargetCreateInfo& info, TargetHandle recycle)
{
	UPtr<DxTarget> newTarget(new DxTarget());
	DxTarget* target = (recycle == (TargetHandle)0) ? newTarget.release() : DxTarget::upcast(recycle);
	target->reset();

	target->viewport.TopLeftX = (FLOAT)info.viewport.x;
	target->viewport.TopLeftY = (FLOAT)info.viewport.y;
	target->viewport.Width = (FLOAT)info.viewport.w;
	target->viewport.Height = (FLOAT)info.viewport.h;
	target->viewport.MaxDepth = 1;
	target->viewport.MinDepth = 0;

	target->scissor.top = info.scissor.y;
	target->scissor.left = info.scissor.x;
	target->scissor.bottom = info.scissor.y + info.scissor.h;
	target->scissor.right = info.scissor.x + info.scissor.w;

	for (size_t i = 0; i < info.attachmentCount; i++)
	{
		TargetView view;
		view.output = DxResource::upcast(info.attachments[i].image);
		view.index = info.attachments[i].index;
		target->renderTargets.push_back(view);
	}

	target->depthStencil.output = DxResource::upcast(info.depth.image);
	target->depthStencil.index = info.depth.index;

	//Prewarm the view cache
	target->warm();

	return RPtr<TargetHandle>(this, DxTarget::downcast(target));
}

void Dx11::destroy(TargetHandle target)
{
	if (DxTarget* t = DxTarget::upcast(target))
	{
		delete t;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
