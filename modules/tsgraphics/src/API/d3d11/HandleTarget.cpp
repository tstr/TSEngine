/*
	Render API

	D3D11Render target handling methods
*/

#include "Render.h"
#include "HandleTarget.h"

using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RPtr<TargetHandle> D3D11::createTarget(const TargetCreateInfo& info, TargetHandle recycle)
{
	D3D11Target* target = (recycle == (TargetHandle)0) ? new D3D11Target() : D3D11Target::upcast(recycle);
	target->reset();

	target->viewport.TopLeftX = (FLOAT)info.viewport.x;
	target->viewport.TopLeftY = (FLOAT)info.viewport.y;
	target->viewport.Width = (FLOAT)info.viewport.w;
	target->viewport.Height = (FLOAT)info.viewport.w;
	target->viewport.MaxDepth = 1;
	target->viewport.MinDepth = -1;

	target->scissor.top = info.scissor.y;
	target->scissor.left = info.scissor.x;
	target->scissor.bottom = info.scissor.y + info.scissor.h;
	target->scissor.right = info.scissor.x + info.scissor.w;

	for (size_t i = 0; i < info.attachmentCount; i++)
	{
		TargetView view;
		view.output = D3D11Resource::upcast(info.attachments[i].image);
		view.index = info.attachments[i].index;
		target->renderTargets.push_back(view);
	}

	target->depthStencil.output = D3D11Resource::upcast(info.depth.image);
	target->depthStencil.index = info.depth.index;

	//Prewarm the view cache
	target->warm();

	return RPtr<TargetHandle>(this, D3D11Target::downcast(target));
}

void D3D11::destroy(TargetHandle target)
{
	if (auto t = D3D11Target::upcast(target))
	{
		delete t;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
