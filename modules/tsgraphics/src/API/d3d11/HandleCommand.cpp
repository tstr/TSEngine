/*
	Render API

	Command object
*/

#include "Render.h"
#include "HandleCommand.h"

using namespace ts;

///////////////////////////////////////////////////////////////////////////////
//  Draw command
///////////////////////////////////////////////////////////////////////////////

RPtr<CommandHandle> D3D11::createCommand(const DrawCommandInfo& info, CommandHandle recycle)
{
	auto r = D3D11DrawCommand::upcast(recycle);
	UPtr<D3D11DrawCommand> cmd((r == nullptr) ? new D3D11DrawCommand() : r);
	
	cmd->pipeline = D3D11Pipeline::upcast(info.pipeline);
	cmd->inputs = D3D11ResourceSet::upcast(info.inputs);
	cmd->outputs = D3D11Target::upcast(info.outputs);

	cmd->count = info.count;
	cmd->start = info.start;
	cmd->vertexBase = info.vbase;
	cmd->instances = info.instances;
	cmd->mode = info.mode;

	return RPtr<CommandHandle>(this, D3D11DrawCommand::downcast(cmd.release()));
}

void D3D11::destroy(CommandHandle cmd)
{
	if (auto d = D3D11DrawCommand::upcast(cmd))
	{
		delete d;
	}
}

///////////////////////////////////////////////////////////////////////////////
