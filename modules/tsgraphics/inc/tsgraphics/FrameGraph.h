/*
    Frame Graph class
*/

#pragma once

#include <tscore/delegate.h>
#include <tscore/table.h>

#include "Driver.h"

namespace ts
{
	enum class SizeClass
	{
		ABSOLUTE,
		RELATIVE,
	};

	struct FrameResourceImageInfo
	{
		SizeClass size;
		float width;
		float height;

		uint32 length;

		ImageFormat format;
	};

	using FrameGraphResource = uint32;

    class FrameGraph
    {
    public:
        
		FrameGraphResource addImage(const char* name, const FrameResourceImageInfo& info);

		template<class Func1, class Func2>
        void addPass(const char* name, Func1&& configureFunc, Func2&& execFunc);

		void compile();

		void execute();

    private:

		void invalidate();

		struct Resource
		{
			RPtr<ResourceHandle> handle;
			FrameResourceImageInfo info;

			uint32 refCount;
		};

		struct Pass
		{
			std::vector<FrameGraphResource> reads;
			std::vector<FrameGraphResource> writes;
		};

		ResourceHandle realizeImage(FrameGraphResource rsc);

		Table<Resource, FrameGraphResource> m_resources;
		std::vector<Pass> m_passes;
    };
}