/*
	Render API

	Target class
*/

#pragma once

#include "Render.h"
#include "Helpers.h"
#include "Handle.h"
#include "HandleResource.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	struct TargetView
	{
		DxResource* output = nullptr;
		uint32 index = 0;
	};

	struct DxTarget : public Handle<DxTarget, TargetHandle>
	{
		std::vector<TargetView> renderTargets;
		TargetView depthStencil;

		D3D11_VIEWPORT viewport;
		D3D11_RECT scissor;

		//Warm the view cache for each rtv+dsv
		void warm()
		{
			for (const TargetView& view : renderTargets)
			{
				if (view.output)
					view.output->getRTV(view.index);
			}

			if (depthStencil.output)
				depthStencil.output->getDSV(depthStencil.index);
		}

		void clearRenderTargets(ID3D11DeviceContext* context, RGBA colour)
		{
			Vector vcolour = colour;

			for (const TargetView& view : renderTargets)
			{
				if (view.output)
					context->ClearRenderTargetView(view.output->getRTV(view.index), (const float*)&vcolour);
			}
		}

		void clearDepthStencil(ID3D11DeviceContext* context, float depth)
		{
			context->ClearDepthStencilView(
				depthStencil.output->getDSV(depthStencil.index),
				D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
				depth,
				0
			);
		}
		
		void bind(ID3D11DeviceContext* context)
		{
			ID3D11RenderTargetView* rtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
			ZeroMemory(rtvs, sizeof(rtvs));

			for (size_t i = 0; i < renderTargets.size(); i++)
			{
				rtvs[i] = nullptr;
				DxResource* output = renderTargets[i].output;
				if (output != nullptr)
					rtvs[i] = output->getRTV(renderTargets[i].index);
			}

			ID3D11DepthStencilView* dsv = nullptr;

			if (depthStencil.output != nullptr)
			{
				dsv = depthStencil.output->getDSV(depthStencil.index);
			}

			context->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, rtvs, dsv);

			context->RSSetViewports(1, &viewport);
			context->RSSetScissorRects(1, &scissor);
		}

		void reset()
		{
			renderTargets.clear();
			depthStencil.output = nullptr;
			ZeroMemory(&scissor, sizeof(scissor));
			ZeroMemory(&viewport, sizeof(viewport));
		}
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
