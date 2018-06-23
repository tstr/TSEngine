/*
	Graphics Command Queue source

	todo: improve memory allocation and overall efficiency
*/

#include <tsgraphics/CommandQueue.h>
#include <tscore/alloc/Linear.h>
#include <tscore/debug/assert.h>

#include <tsgraphics/Driver.h>

#include <algorithm>

using namespace std;
using namespace ts;

///////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	/*
		Command Batch struct:

		Implemented as a linked list of command structs.
	*/
	struct CommandBatch
	{
		Command* first;
	};

	/*
		Command struct:

		An individual link in a Command Batch,
		Contains a callable command dispatcher.
	*/
	struct Command
	{
		Command* next;
		CommandDelegate dispatchFunc;
		uint32 dispatchSize = 0;
		uint32 extraSize = 0;
	};

}

/*
	Commmand Batch Key struct:

	Contains a sort key, batch pointer and comparison operator.
*/
struct SBatchKey
{
	CommandQueue::SortKey key;
	CommandBatch* batch;

	bool operator<(const SBatchKey& pair)
	{
		return less<CommandQueue::SortKey>()(key, pair.key);
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////
// CommandQueue implementation
///////////////////////////////////////////////////////////////////////////////////////////////
class CommandQueue::Queue : private LinearAllocator
{
private:

	//Internal allocators - share the same chunk of memory
	LinearAllocator m_keyAllocator;
	LinearAllocator m_batchAllocator;

	size_t m_batchCapacity;

public:

	Queue(uint32 numBatches) :
		m_batchCapacity(numBatches)
	{
		size_t initialCapacity = 0;
		initialCapacity += numBatches * sizeof(Command) * 16;	// Each batch has enough capacity for 16 commands
		initialCapacity += 2 * 1024 * 1024;						// 2MBs for extra memory

		//Call base constructor
		this->LinearAllocator::LinearAllocator(initialCapacity);

		//Reserve chunk for key allocator
		this->alloc(numBatches * sizeof(SBatchKey));
		
		m_keyAllocator.resize(this->getStart(), this->getTop());
		m_batchAllocator.resize(this->getTop(), this->getEnd());
	}

	~Queue()
	{
		//this->LinearAllocator::~LinearAllocator();
	}

	//Allocate Command Batch from allocator
	CommandBatch* allocBatch()
	{
		return m_batchAllocator.alloc<CommandBatch>();
	}

	//Allocate Command from allocator
	Command* allocCommand(size_t cmdSize)
	{
		return (Command*)m_batchAllocator.alloc(cmdSize);
	}

	//Allocate a Command Batch Key pair
	void addKey(CommandQueue::SortKey key, CommandBatch* batch)
	{
		SBatchKey* pair = m_keyAllocator.alloc<SBatchKey>(1, 1);
		tsassert(pair != nullptr);

		pair->batch = batch;
		pair->key = key;
	}

	//Get pointer to first key
	SBatchKey* beginKey()
	{
		return (SBatchKey*)m_keyAllocator.getStart();
	}

	//Get pointer to end key
	SBatchKey* endKey()
	{
		return (SBatchKey*)m_keyAllocator.getTop();
	}

	//Get number of allocated Command Batch Keys
	size_t getKeyCount() const
	{
		auto top = (size_t)m_keyAllocator.getTop();
		auto start = (size_t)m_keyAllocator.getStart();

		return (top - start) / sizeof(SBatchKey);
	}

	//Get number of allowed batches
	size_t getBatchCapacity() const
	{
		return m_batchCapacity;
	}

	//Reset key and batch allocators
	void reset()
	{
		m_keyAllocator.reset();
		m_batchAllocator.reset();
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////
// ctor/dtor
///////////////////////////////////////////////////////////////////////////////////////////////

CommandQueue::CommandQueue(uint32 numBatches) :
	pQueue(new CommandQueue::Queue(numBatches))
{}

CommandQueue::~CommandQueue()
{
	pQueue.reset();
}

///////////////////////////////////////////////////////////////////////////////////////////////
//	Batch methods
///////////////////////////////////////////////////////////////////////////////////////////////

//Allocate an empty command batch
CommandBatch* CommandQueue::createBatch()
{
	tsassert(pQueue);

	CommandBatch* b = pQueue->allocBatch();
	tsassert(b != nullptr);

	tsassert(pQueue->getKeyCount() < pQueue->getBatchCapacity());

	memset(b, 0, sizeof(CommandBatch));

	return b;
}

//Enqueue a command batch
void CommandQueue::submitBatch(SortKey key, CommandBatch* batch)
{
	tsassert(pQueue);

	pQueue->addKey(key, batch);
}

///////////////////////////////////////////////////////////////////////////////////////////////

//Execute the dispatcher of a given command
static void executeCommand(RenderContext* context, Command* cmd)
{
	if (cmd != nullptr)
	{
		//Get pointer to extra parameters
		const byte* extra = (const byte*)cmd + sizeof(Command) + cmd->dispatchSize;

		//Call dispatcher
		cmd->dispatchFunc(context, (const void*)extra);

		//Recursively dispatch the next command
		executeCommand(context, cmd->next);
	}
}

//Execute queued command batches
void CommandQueue::flush(RenderContext* context)
{
	tsassert(pQueue);

	//For each key
	for (SBatchKey* pair = pQueue->beginKey(); pair != pQueue->endKey(); pair++)
	{
		//Get pointer to command batch
		CommandBatch* batch = pair->batch;
		//Recursively execute each command in this batch
		executeCommand(context, batch->first);
	}

	//Clear allocators
	pQueue->reset();
}

//Sort queued command batches based on their keys
void CommandQueue::sort()
{
	tsassert(pQueue);

	//Sort array of Command Batch Key pairs
	std::sort(
		pQueue->beginKey(),
		pQueue->endKey()
	);
}

///////////////////////////////////////////////////////////////////////////////////////////////
//	Internal command management methods
///////////////////////////////////////////////////////////////////////////////////////////////

//Allocate a command block capable of storing a command dispatcher + any extra parameters
Command* CommandQueue::commandAlloc(size_t dispatchSize, size_t extraSize)
{
	tsassert(pQueue);

	const size_t totalSize = sizeof(Command) + dispatchSize + extraSize;
	//Request memory from the batch pool
	Command* pCmd = pQueue->allocCommand(totalSize);

	//If pointer is null then we have run out of memory
	tsassert(pCmd != nullptr);

	//Zero out data
	memset(pCmd, 0, totalSize);

	pCmd->dispatchSize = (uint32)dispatchSize;
	pCmd->extraSize = (uint32)extraSize;

	return (Command*)pCmd;
}

//Attaches an individual command to a batch for execution
void CommandQueue::commandAttach(CommandBatch* pBatch, Command* pCmd)
{
	tsassert(pQueue);

	//Appends a command to the command list of this batch

	//If command batch is empty
	if (pBatch->first == nullptr)
	{
		//Set the first command to be this command
		pBatch->first = pCmd;
	}
	else
	{
		Command* cur = pBatch->first;

		//Iterate over each command until we reach the end of the list
		while (cur->next != nullptr)
		{
			cur = cur->next;
		}

		//Set the command to be the end of the list
		cur->next = pCmd;
	}
}

//Copy a given dispatcher into the command block
void* CommandQueue::storeCommandDispatcher(Command* pCmd, const void* pDispatchSrc)
{
	tsassert(pQueue);

	if ((pCmd == nullptr) || (pCmd->dispatchSize == 0))
	{
		return nullptr;
	}

	//Obtain address of new dispatcher
	void* dispatchDest = (byte*)pCmd + sizeof(Command);

	//Copy old dispatcher into the new dispatcher
	memcpy(dispatchDest, pDispatchSrc, pCmd->dispatchSize);

	//Return address of new dispatcher
	return dispatchDest;
}

//Copy a given dispatcher delegate into the command block
void CommandQueue::storeCommandDispatcherFunc(Command* pCmd, CommandDelegate dispatcherFunc)
{
	tsassert(pQueue);

	if ((pCmd == nullptr) || (pCmd->dispatchSize == 0))
	{
		return;
	}

	//Set dispatcher 
	pCmd->dispatchFunc = dispatcherFunc;
}

//Copy a given block of memory into the command block
void CommandQueue::storeCommandExtra(Command* pCmd, const void* pExtraSrc)
{
	tsassert(pQueue);

	if ((pCmd == nullptr) || (pExtraSrc == nullptr) || (pCmd->dispatchSize == 0))
	{
		return;
	}

	//Get pointer to extra memory
	void* extraDest = (byte*)pCmd + sizeof(Command) + pCmd->dispatchSize;

	//Only copy extra parameters into this block if extra memory has been given
	if (pCmd->extraSize > 0)
	{
		memcpy(extraDest, pExtraSrc, pCmd->extraSize);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
