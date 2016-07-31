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

#define PROFILE_BLOCK_BEGIN ts::Stopwatch __TIMER; __TIMER.start();
#define PROFILE_BLOCK_END __TIMER.stop(); tslog(__TIMER.deltaTime(), ts::ELogLevel::eLevelProfile);

#endif