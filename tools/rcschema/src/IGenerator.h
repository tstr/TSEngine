/*
	Schema generator
*/

#include "Schema.h"

#include <tscore/path.h>

namespace ts
{
	namespace rc
	{
		class IGenerator
		{
		public:
			
			virtual bool generate(const Schema& schema, const Path& outputdir = "") = 0;
		};
	}
}
