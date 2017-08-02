/*
	Schema parser class source
*/

#include "SchemaReader.h"

#include <iostream>
#include <fstream>

using namespace ts;
using namespace ts::rc;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Token structure
*/
struct Token
{
	//Token data
	String data;
	uint32 line;

	//Constructors
	Token() : line(0) {}
	Token(const String& tok, uint32 l = 0) : data(tok), line(l) {}

	operator String() const { return data; }

	//Operators
	bool operator==(const char* str) const { return data == str; }
	bool operator==(const String& str) const { return data == str; }

	bool operator!=(const char* str) const { return !(*this == str); }
	bool operator!=(const String& str) const { return !(*this == str); }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Parser exception
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ParserException : public std::exception
{
private:

	String m_msg;

public:

	ParserException(const Token& tokenUnexpect, const String& tokenExpect)
	{
		m_msg = format("[line: %] Unexpected token \"%\" expected \"%\"", tokenUnexpect.line, tokenUnexpect.data, tokenExpect);
	}

	const char* what() const override
	{
		return m_msg.c_str();
	}
};

typedef std::exception Exception;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Token classes
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	TokenList class:

	Represents a sequence of tokens.
*/
class SchemaReader::TokenList : private std::vector<Token>
{
public:

	using Base = std::vector<Token>;
	using Iterator = typename Base::const_iterator;

	TokenList()
	{
		Base::push_back(Token());
	}

	void add(const Token& tok)
	{
		Base::push_back(tok);
	}

	Iterator begin() const
	{
		return Base::begin();
	}

	Iterator end() const
	{
		return Base::end();
	}

	void reset()
	{
		this->clear();
	}
};

/*
	TokenParser class:

	Processes a sequence of tokens.
*/
class SchemaReader::TokenParser
{
private:

	TokenList m_tokens;

	TokenList::Iterator m_tokenIter;

	Token m_nullToken;

public:

	//Constructor
	TokenParser(const TokenList& list) :
		m_tokens(list),
		m_tokenIter(m_tokens.begin())
	{}

	/*
		Token methods
	*/

	//Return true if there is a next token in the sequence
	bool hasNext() const
	{
		auto it = m_tokenIter;
		it++;
		return it != m_tokens.end();
	}

	//Peek current token in sequence
	const Token& current() const
	{
		//Check bounds
		if (m_tokenIter == m_tokens.end())
		{
			return m_nullToken;
		}
		else
		{
			return *m_tokenIter;
		}
	}

	//Peek next token in sequence
	const Token& peek()
	{
		//Check bounds
		if (m_tokenIter == m_tokens.end())
		{
			return m_nullToken;
		}
		else
		{
			//Copy current iterator
			auto it = m_tokenIter;
			return *(++it);
		}
	}

	//Return next token in sequence and increment token counter
	const Token& next()
	{
		if (m_tokenIter == m_tokens.end())
		{
			return m_nullToken;
		}
		else
		{
			return *(++m_tokenIter);
		}
	}

	/*
		Static methods:

		Character classes
	*/
	static bool isSpace(char c)
	{
		return ((c == ' ') || (c == '\r') || (c == '\n') || (c == '\t'));
	}

	static bool isLetter(char c)
	{
		return ((c <= 'z') && (c >= 'a')) || ((c <= 'Z') && (c >= 'A'));
	}

	static bool isNumber(char c)
	{
		return ((c <= '9') && (c >= '0'));
	}

	static bool isSymbol(char c)
	{
		return (c == ';') || (c == '{') || (c == '}') || (c == ',');
	}

	/*
		Static methods:
	*/
	static bool isIdentifier(const Token& identifier)
	{
		bool is = !isNumber(identifier.data.front());

		for (char c : identifier.data)
		{
			is = is && (isNumber(c) || isLetter(c) || (c == '_'));
		}

		return is;
	}
};



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SchemaReader::read(const Path& filePath, Schema& schema)
{
	//Schema file path
	ifstream schemaFile(filePath.str());

	if (schemaFile.fail())
	{
		cerr << "ERROR: Unable to read file \"" << filePath.str() << "\"\n";
		return false;
	}

	const String schemaName(filePath.str());

	//Trim file extension and directory from file name
	size_t splitPos = schemaName.find_last_of('/');
	splitPos = (splitPos == String::npos) ? 0 : splitPos;
	size_t splitLen = schemaName.find_last_of('.') - splitPos;

	//Set schema name based on schema file name
	schema.setName(schemaName.substr(splitPos, splitLen));

	TokenList tokens;
	tokenize(schemaFile, tokens);
	
	try
	{
		parse(tokens, schema);
	}
	catch (ParserException e)
	{
		cerr << "ERROR: [file: \"" << filePath.str() << "\"]" << e.what() << endl;
		schema.clear();
		return false;
	}
	catch (...)
	{
		//Exit quietly
		schema.clear();
		return false;
	}

	return true;
}

/*
	Tokens are split on whitespace or symbols: ";" "{" "}"
*/
void SchemaReader::tokenize(istream& characters, TokenList& tokens)
{
	String curToken;

	//Previous character
	char prevChar = ' ';
	char curChar = 0;
	uint32 curLine = 1;

	bool isComment = false;

	while(characters.read(&curChar, 1))
	{
		if (isComment)
		{
			//Newline character marks end of comment
			if (curChar == '\n')
			{
				isComment = false;
			}
		}
		else
		{
			//If current character marks start of comment
			if (curChar == '#')
			{
				isComment = true;
			}
			else
			{
				//Process characters

				if (!TokenParser::isSpace(curChar))
				{
					if (TokenParser::isSpace(prevChar) || TokenParser::isSymbol(prevChar) || (!TokenParser::isSpace(prevChar) && TokenParser::isSymbol(curChar)))
					{
						if (!curToken.empty())
						{
							tokens.add(Token(curToken, curLine));
							//cout << format("Token(%), Line(%)\n", curToken, curLine);
							curToken.clear();
						}
					}

					curToken += curChar;
				}
			}
		}

		//todo: fix bug where tokens that are at the end of the line appear on the next line
		if (curChar == '\n')
		{
			curLine++;
		}

		prevChar = curChar;
	}

	tokens.add(curToken);
}

/*
	Process tokens according to definition:

	decl ::= <type> <identifier>; <decl>
	schema ::= resource <identifier> { <decl> } <schema>
*/
void SchemaReader::parse(const TokenList& tokens, Schema& schema)
{
	/*
	//debug print tokens
	cout << "-----------------------------------\n";
	for (const String& tok : tokens)
	{
		cout << "[" << tok << "]\n";
	}
	cout << "-----------------------------------\n";
	//*/

	TokenParser parser(tokens);

	while (parser.hasNext())
	{
		//Expected declaration
		const String& declaration = parser.next();

		//Resource declaration
		if (declaration == "resource")
		{
			parseResource(parser, schema);
		}
		//Inline struct declaration
		else if (declaration == "data")
		{
			parseDataType(parser, schema);
		}
		//Enum type declaration
		else if (declaration == "enum")
		{
			parseEnumType(parser, schema);
		}
		else
		{
			throw ParserException(declaration, "<DECLARATION>");
		}
	}
}

/*
	Parse resource declaration
*/
void SchemaReader::parseResource(TokenParser& parser, Schema& schema)
{
	//Expected resource identifier
	if (TokenParser::isIdentifier(parser.next()))
	{
		Resource curRsc = Resource(&schema, parser.current());

		//Parse field set
		parseFields(parser, curRsc.getFields());

		//Save schema
		if (!schema.addResource(curRsc))
		{
			cerr << "ERROR: Unable to define resource \"" << curRsc.getName() << "\"\n";
			throw Exception();
		}
	}
	else
	{
		throw ParserException(parser.current(), "<IDENTIFIER>");
	}
}

/*
	Parse user defined data type
*/
void SchemaReader::parseDataType(TokenParser& parser, Schema& schema)
{
	//If is identifier
	if (TokenParser::isIdentifier(parser.next()))
	{
		const String& dataTypeName = parser.current();
		FieldSet dataTypeFields(&schema);

		//Parse field set
		parseFields(parser, dataTypeFields);

		//Save data type
		if (!schema.defineType(dataTypeName, dataTypeFields))
		{
			cerr << "ERROR: Unable to define data type \"" << dataTypeName << "\"\n";
			throw Exception();
		}
	}
	else
	{
		throw ParserException(parser.current(), "<IDENTIFIER>");
	}
}

/*
	Parse user defined enum type
*/
void SchemaReader::parseEnumType(TokenParser& parser, Schema& schema)
{
	//If is identifier
	if (TokenParser::isIdentifier(parser.next()))
	{
		const String& enumTypeName = parser.current();
		Schema::EnumSet enumSet;

		//Expected open brace
		if (parser.next() == "{")
		{
			//While the next token does not equal a closing brace
			while (parser.hasNext())
			{
				//If next token is a closing brace
				if (parser.peek() == "}")
				{
					//Exit loop
					parser.next();
					break;
				}

				//Expected enum value
				if (TokenParser::isIdentifier(parser.peek()))
				{
					enumSet.push_back(parser.next());

					//Expected end of value statement this is the last enum value
					if (parser.next() != ",")
					{
						if (parser.current() == "}")
						{
							break;
						}
						else
						{
							throw ParserException(parser.current(), ",");
						}
					}
				}
				else
				{
					throw ParserException(parser.current(), "<IDENTIFIER>");
				}
			}

			//Save enum type
			if (!schema.defineEnum(enumTypeName, enumSet))
			{
				cerr << "ERROR: Unable to define enum type \"" << enumTypeName << "\"\n";
				throw Exception();
			}
		}
		else
		{
			throw ParserException(parser.current(), "{");
		}
	}
	else
	{
		throw ParserException(parser.current(), "<IDENTIFIER>");
	}
}

/*
	Parse field set from tokens.
	Format:

	{ <field-entry0>; <field-entry1>; ... ; }
*/
void SchemaReader::parseFields(TokenParser& parser, FieldSet& fields)
{
	//Expected open brace
	if (parser.next() == "{")
	{
		//While the next token does not equal a closing brace
		while (parser.hasNext())
		{
			//If next token is a closing brace
			if (parser.peek() == "}")
			{
				//Exit loop
				parser.next();
				break;
			}

			String fieldName;
			String fieldType;

			//Expected type
			if (fields.getSchema()->isType(fieldType = parser.next()))
			{
				//Expected identifier
				if (TokenParser::isIdentifier(fieldName = parser.next()))
				{
					//Expected end of statement
					if (parser.next() != ";")
					{
						throw ParserException(parser.current(), ";");
					}
				}
				else
				{
					throw ParserException(fieldName, "<IDENTIFIER>");
				}
			}
			else
			{
				cerr << "ERROR: \"" << fieldType << "\" is not a valid type name\n";
				throw Exception();
			}

			//Save field
			if (!fields.add(fieldName, fieldType))
			{
				cerr << "ERROR: unable to add field: type=\"" << fieldType << "\", name=\"" << fieldName << "\"\n";
				throw Exception();
			}
		}
	}
	else
	{
		throw ParserException(parser.current(), "{");
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
