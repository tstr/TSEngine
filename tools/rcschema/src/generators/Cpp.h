/*
	C++ code generator
*/

#include "../IGenerator.h"

namespace ts
{
	namespace rc
	{
		class CPPGenerator : public IGenerator
		{
		private:

			// Internal generator methods

			void generateBuilderClass(std::ostream& file, const Resource& rsc);
			void generateLoaderClass(std::ostream& file, const Resource& rsc);

			size_t generateFieldDescriptorTable(std::ostream& file, const Resource& rcs);

			void generateFieldSetter(std::ostream& file, const Field& field);
			void generateFieldGetter(std::ostream& file, const Field& field);

		public:

			CPPGenerator() = default;

			bool generate(const Schema& schema, const Path& outputdir, uint32 flags) override;
		};
	}
}
