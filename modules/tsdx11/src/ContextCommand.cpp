/*
	Render API

	Draw command methods
*/

#include "Context.h"

#include "HandleTarget.h"
#include "HandleResourceSet.h"
#include "HandlePipeline.h"

using namespace ts;

///////////////////////////////////////////////////////////////////////////////

/*
	Draw call mapping table
	Alternative to using switch statement
*/
static void drawCallSig(ID3D11DeviceContext*, const DrawParams&);
using DrawCaller = decltype(drawCallSig)*;

DrawCaller drawFunctions(DrawMode mode)
{
	static DrawCaller table[4];

	table[(size_t)DrawMode::VERTEX] = [](auto ctx, auto cmd)
	{
		ctx->Draw(cmd.count, cmd.start);
	};

	table[(size_t)DrawMode::INDEXED] = [](auto ctx, auto cmd)
	{
		ctx->DrawIndexed(cmd.count, cmd.start, cmd.vbase);
	};

	table[(size_t)DrawMode::INSTANCED] = [](auto ctx, auto cmd)
	{
		ctx->DrawInstanced(cmd.count, cmd.instances, cmd.start, 0);
	};

	table[(size_t)DrawMode::INDEXEDINSTANCED] = [](auto ctx, auto cmd)
	{
		ctx->DrawIndexedInstanced(cmd.count, cmd.instances, cmd.start, cmd.vbase, 0);
	};

	return table[(size_t)mode];
}

///////////////////////////////////////////////////////////////////////////////

void Dx11Context::draw(TargetHandle outputs, PipelineHandle pipeline, ResourceSetHandle inputs, const DrawParams& params)
{
	auto ctx = m_context.Get();

	//Bind input/output resources and pipeline state
	DxPipeline::upcast(pipeline)->bind(ctx);
	DxTarget::upcast(outputs)->bind(ctx);
	DxResourceSet::upcast(inputs)->bind(ctx);

	//Lookup draw call function in table
	//And call it
	drawFunctions(params.mode)(ctx, params);

	//For debugging
	m_driver->incrementDrawCallCounter();
}

///////////////////////////////////////////////////////////////////////////////
