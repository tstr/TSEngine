/*
	Schema generator
*/

#include "Schema.h"

#include <tscore/path.h>

namespace ts
{
	namespace rc
	{
		enum EGeneratorFlags : uint32
		{
			GENERATE_BUILDER = 1,
			GENERATE_LOADER  = 2,
			GENERATE_ALL	 = GENERATE_BUILDER | GENERATE_LOADER
		};

		class IGenerator
		{
		public:
			
			virtual bool generate(const Schema& schema, const Path& outputdir = "", uint32 flags = GENERATE_ALL) = 0;
		};
	}
}
