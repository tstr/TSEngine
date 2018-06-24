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

RPtr<CommandHandle> Dx11::createCommand(const DrawCommandInfo& info, CommandHandle recycle)
{
	auto r = DxDrawCommand::upcast(recycle);
	UPtr<DxDrawCommand> cmd((r == nullptr) ? new DxDrawCommand() : r);
	
	cmd->pipeline = DxPipeline::upcast(info.pipeline);
	cmd->inputs = DxResourceSet::upcast(info.inputs);
	cmd->outputs = DxTarget::upcast(info.outputs);

	cmd->count = info.count;
	cmd->start = info.start;
	cmd->vertexBase = info.vbase;
	cmd->instances = info.instances;
	cmd->mode = info.mode;

	return RPtr<CommandHandle>(this, DxDrawCommand::downcast(cmd.release()));
}

void Dx11::destroy(CommandHandle cmd)
{
	if (auto d = DxDrawCommand::upcast(cmd))
	{
		delete d;
	}
}

///////////////////////////////////////////////////////////////////////////////
