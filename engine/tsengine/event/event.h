/*
	Event system
*/

#pragma once

#include <tscore/types.h>

namespace ts
{
	enum EEventCode
	{
	
	};
	
	struct SEvent
	{
		EEventCode code;
		
		//data
	};
	
	class IEventListener;
	
	class CEventDispatcher
	{
	public:
		
		void post(EEventCode event);
		
		void addListener(IEventListener* ls);
		
		//message loop
		void process();
	};
	
	class IEventListener
	{
	public:
		
		virtual void onEvent(EEventCode e) = 0;
	};
}

/*

{
	gSystem->getEventDispatcher()->post(eEventPreRender)
}

*/