/*
	Render API

	Draw command methods
*/

#include "Context.h"
#include "HandleCommand.h"

using namespace ts;

///////////////////////////////////////////////////////////////////////////////

/*
	Draw call mapping table
	Alternative to using switch statement
*/
static void drawCallSig(ID3D11DeviceContext*, DxDrawCommand*);
using DrawCaller = decltype(drawCallSig)*;

DrawCaller drawFunctions(DrawMode mode)
{
	static DrawCaller table[4];

	table[(size_t)DrawMode::VERTEX] = [](auto ctx, auto cmd)
	{
		ctx->Draw(cmd->count, cmd->start);
	};

	table[(size_t)DrawMode::INDEXED] = [](auto ctx, auto cmd)
	{
		ctx->DrawIndexed(cmd->count, cmd->start, cmd->vertexBase);
	};

	table[(size_t)DrawMode::INSTANCED] = [](auto ctx, auto cmd)
	{
		ctx->DrawInstanced(cmd->count, cmd->instances, cmd->start, 0);
	};

	table[(size_t)DrawMode::INDEXEDINSTANCED] = [](auto ctx, auto cmd)
	{
		ctx->DrawIndexedInstanced(cmd->count, cmd->instances, cmd->start, cmd->vertexBase, 0);
	};

	return table[(size_t)mode];
}

///////////////////////////////////////////////////////////////////////////////

void Dx11Context::submit(CommandHandle command)
{
	auto ctx = m_context.Get();

	if (auto cmd = DxDrawCommand::upcast(command))
	{
		//Bind input/output resources and pipeline state
		cmd->pipeline->bind(ctx);
		cmd->inputs->bind(ctx);
		cmd->outputs->bind(ctx);

		//Lookup draw call function in table
		//And call it
		drawFunctions(cmd->mode)(ctx, cmd);
	}

	//For debugging
	m_driver->incrementDrawCallCounter();
}

///////////////////////////////////////////////////////////////////////////////
