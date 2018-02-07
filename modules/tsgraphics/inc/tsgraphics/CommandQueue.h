/*
	Graphics Command Queue:
	
	Queue class for stateless submission and sorting of Command Batches.
	Command Batches consist of 1 or more commands, which themselves contain generic command dispatchers for executing Low Level Render Api Context commands.

	This header contains several common command dispatchers.
*/

#pragma once

#include <tsgraphics/abi.h>

#include <tscore/types.h>
#include <tscore/ptr.h>
#include <tscore/delegate.h>
#include <tsgraphics/GraphicsCore.h>

namespace ts
{
	struct Command;
	struct CommandBatch;

	typedef const void* CommandPtr;
	typedef Delegate<void(IRenderContext*, CommandPtr)> CommandDelegate;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
		Command Queue class
	*/
	class CommandQueue
	{
	private:
		
		//Implementation
		class Queue;
		OpaquePtr<Queue> pQueue;

		//Internal command management methods
		TSGRAPHICS_API Command* commandAlloc(size_t dispatchSize, size_t extraSize);
		TSGRAPHICS_API void commandAttach(CommandBatch* pBatch, Command* pCmd);
		TSGRAPHICS_API void* storeCommandDispatcher(Command* pCmd, const void* pDispatchSrc);
		TSGRAPHICS_API void storeCommandDispatcherFunc(Command* pCmd, CommandDelegate dispatcherFunc);
		TSGRAPHICS_API void storeCommandExtra(Command* pCmd, const void* pExtraSrc);

		/*
			Asserts that comand dispatcher(and extra parameter) is a POD type
		*/
		template<typename dispatcher_t, typename param_t = dispatcher_t>
		struct VerifyDispatcher
		{
			static_assert(std::is_pod<dispatcher_t>::value, "Command Dispatcher must be a POD type");
			static_assert(std::is_pod<param_t>::value, "Command Parameter must be a POD type");
		};

	public:

		OPAQUE_PTR(CommandQueue, pQueue)

		typedef uint64 SortKey;

		//Ctor/dtor
		CommandQueue() {}
		TSGRAPHICS_API CommandQueue(uint32 numBatches);
		TSGRAPHICS_API ~CommandQueue();

		//Create new command batch for queueing.
		TSGRAPHICS_API CommandBatch* createBatch();
		
		//Submit command batch to queue.
		TSGRAPHICS_API void submitBatch(SortKey key, CommandBatch* batch);

		//////////////////////////////////////////////////////////////////////////////////
		/*
			Add a command dispatcher to a batch:

			- Command dispatchers must implement a non static method called dispatch() with the signature void(IRenderContext*, CommandPtr).
			- Command dispatchers must be POD types.

			- When a Command Batch is executed it's dispatchers will be executed in the order that they are added.
		*/
		template<typename dispatcher_t, typename = VerifyDispatcher<dispatcher_t>>
		void addCommand(CommandBatch* pBatch, const dispatcher_t& disp, const void* extraData = nullptr, size_t extraSize = 0)
		{
			//Allocate enough memory for the command dispatcher and any other parameters
			Command* pCmd = commandAlloc(sizeof(dispatcher_t), extraSize);

			//Set command dispatcher
			//Return value is address of dispatcher copy
			void* pDisp = storeCommandDispatcher(pCmd, &disp);
			//Set dispatcher function - it is called using pDisp as an argument
			storeCommandDispatcherFunc(pCmd, CommandDelegate::fromMethod<dispatcher_t, &dispatcher_t::dispatch>((dispatcher_t*)pDisp));
			//Set any extra parameters
			storeCommandExtra(pCmd, extraData);

			//Attach this command to a batch
			commandAttach(pBatch, pCmd);
		}

		//Add command with extra data of a given type
		template<
			typename dispatcher_t,
			typename param_t,
			typename = VerifyDispatcher<dispatcher_t, param_t>
		>
		void addCommand(CommandBatch* batch, const dispatcher_t& disp, const param_t& param)
		{
			this->addCommand(batch, disp, (const void*)&param, sizeof(param_t));
		}

		//Add command with an array of extra data of a given type
		template<
			typename dispatcher_t,
			typename param_t,
			typename = VerifyDispatcher<dispatcher_t, param_t>
		>
		void addCommand(CommandBatch* batch, const dispatcher_t& disp, param_t* param, size_t paramCount)
		{
			this->addCommand(batch, disp, (const void*)param, sizeof(param_t) * paramCount);
		}

		//////////////////////////////////////////////////////////////////////////////////

		//Sort queued command batches based on their keys
		TSGRAPHICS_API void sort();
		//Execute queued command batches on a given context
		TSGRAPHICS_API void flush(IRenderContext* context);
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
		Command dispatchers
	*/
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//Executes a draw call on a given context
	struct CommandDraw
	{
		HTarget drawTarget;
		HDrawCmd drawCmd;
		SViewport drawView;
		SViewport drawScissor;

		CommandDraw(HTarget target, HDrawCmd cmd, SViewport view, SViewport scissor) :
			drawTarget(target),
			drawCmd(cmd),
			drawView(view),
			drawScissor(scissor)
		{}

		TSGRAPHICS_API void dispatch(IRenderContext* context, CommandPtr extra);
	};

	//Updates a buffer resource on a given context
	struct CommandBufferUpdate
	{
		HBuffer hBuf;

		CommandBufferUpdate() {}
		CommandBufferUpdate(HBuffer b) : hBuf(b) {}

		TSGRAPHICS_API void dispatch(IRenderContext* context, CommandPtr extra);
	};

	//Updates a texture resource on a given context
	struct CommandTextureUpdate
	{
		HTexture hTex;
		uint32 texIdx;

		CommandTextureUpdate() {}
		CommandTextureUpdate(HTexture t, uint32 idx) : hTex(t), texIdx(idx) {}

		TSGRAPHICS_API void dispatch(IRenderContext* context, CommandPtr extra);
	};

	//Clears a target on a given context
	struct CommandTargetClear
	{
		HTarget hTarget;
		Vector colour;
		float depth;

		CommandTargetClear() {}
		CommandTargetClear(HTarget target, const Vector& colour, float depth) :
			hTarget(target),
			colour(colour),
			depth(depth)
		{}

		TSGRAPHICS_API void dispatch(IRenderContext* context, CommandPtr extra);
	};

	//Resolves a multisampled texture on a given context
	struct CommandTextureResolve
	{
		HTexture hDst;
		HTexture hSrc;

		CommandTextureResolve() {}
		CommandTextureResolve(HTexture src, HTexture dest) : hDst(dest), hSrc(src) {}

		TSGRAPHICS_API void dispatch(IRenderContext* context, CommandPtr extra);
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
