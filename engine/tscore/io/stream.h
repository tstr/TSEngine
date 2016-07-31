/*
	String stream header
*/

#pragma once

#include <sstream>
#include <tscore/types.h>

namespace ts
{
	class IStream
	{
	public:
		
		virtual void write(const byte* data, size_t size) = 0;
		virtual void read(byte* data, size_t size) = 0;
	};
}