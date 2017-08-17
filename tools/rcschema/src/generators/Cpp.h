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

			uint32 generateFieldTable(std::ostream& file, const FieldSet& fields);

			void generateFieldSetter(std::ostream& file, const Field& field);
			void generateFieldGetter(std::ostream& file, const Field& field);

			//Get the corresponding CPP type for a given field
			String translateFieldType(const Field& field) const;

		public:

			CPPGenerator() = default;

			bool generate(const Schema& schema, const Path& outputdir, uint32 flags) override;
		};
	}
}
