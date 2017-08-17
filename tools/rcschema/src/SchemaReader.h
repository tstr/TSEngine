/*
	Schema reader class
*/

#pragma once

#include "Schema.h"

#include <tscore/path.h>

namespace ts
{
	namespace rc
	{
		using std::istream;

		/*
			Schema Reader class:
			
			Reads a sequence of characters and returns a set of resource schema structures
		*/
		class SchemaReader
		{
		public:

			//Parse sequence of characters
			bool read(const Path& filePath, Schema& schema);

		private:

			class TokenParser;
			class TokenList;

			//Tokenize character stream
			void tokenize(istream& characters, TokenList& tokens);
			
			//Process tokens
			void parse(const TokenList& tokens, Schema& schema);
			void parseResource(TokenParser& parser, Schema& schema);
			void parseDataType(TokenParser& parser, Schema& schema);
			void parseEnumType(TokenParser& parser, Schema& schema);
			void parseFields(TokenParser& parser, FieldSet& fields, bool allowReferences = true);
		};
	}
}
