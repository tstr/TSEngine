/*
	Profiling header
*/

#pragma once

#include <tscore/system/time.h>
#include <tscore/debug/log.h>

#if !defined _DEBUG
#define TS_NO_PROFILING
#endif

#ifdef TS_NO_PROFILING

#define PROFILE_BLOCK_BEGIN
#define PROFILE_BLOCK_END

#else

#define PROFILE_BLOCK_BEGIN ts::Stopwatch __TIMER__; tsprofile("BEGIN BLOCK PROFILE"); __TIMER__.start();
#define PROFILE_BLOCK_END __TIMER__.stop(); tsprofile("END BLOCK PROFILE: %ms", __TIMER__.deltaTime());

#endif