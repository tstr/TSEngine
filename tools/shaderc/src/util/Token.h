/*
	Token structure: 
	
	Represents a small sequence of text and a type
*/

#pragma once

#include <tscore/strings.h>

namespace ts
{
	enum ECharClass : char
	{
		CHAR_CLASS_OTHER,
		CHAR_CLASS_LETTER,
		CHAR_CLASS_DIGIT,
		CHAR_CLASS_WHITESPACE,
	};

	/*
	Char class lookup table
	*/
	class CharTypes
	{
	private:

		enum { CLASS_TABLE_SIZE = 256 };

		//Class table
		ECharClass m_table[CLASS_TABLE_SIZE];

		CharTypes()
		{
			//Other class
			memset(m_table, CHAR_CLASS_OTHER, CLASS_TABLE_SIZE);

			//Letter class
			for (uint32 i = 0; i <= ('z' - 'a'); i++)
			{
				m_table[(uint32)'a' + i] = CHAR_CLASS_LETTER;
				m_table[(uint32)'A' + i] = CHAR_CLASS_LETTER;
			}

			//Digit class
			for (char c = '0'; c <= '9'; c++)
			{
				m_table[c] = CHAR_CLASS_DIGIT;
			}

			//Whitespace class
			m_table[' '] = CHAR_CLASS_WHITESPACE;
			m_table['\t'] = CHAR_CLASS_WHITESPACE;
			m_table['\n'] = CHAR_CLASS_WHITESPACE;
			m_table['\r'] = CHAR_CLASS_WHITESPACE;
		}

	public:

		static ECharClass get(char c)
		{
			static const CharTypes s_types;
			return s_types.m_table[c];
		}
	};

	/*
	Semantic code
	*/
	enum ETokenCode : uint32
	{
		TOKEN_EMPTY,
		TOKEN_IDENTIFIER,
		TOKEN_INTEGER,
		TOKEN_FLOAT,
		TOKEN_STRING,

		TOKEN_STRUCT,		 // "struct"
		TOKEN_CBUFFER,		 // "cbuffer|tbuffer"
		TOKEN_REGISTER,		 // "register"

		TOKEN_SEMICOLON,	 //;
		TOKEN_COLON,		 //:
		TOKEN_DOT,			 //.
		TOKEN_EQUALS,		 //=
		TOKEN_COMMA,		 //,
		TOKEN_HASH,			 //#

		TOKEN_BLOCK_OPEN,	 //{
		TOKEN_BLOCK_CLOSE,	 //}
		TOKEN_BRACKET_OPEN,  //(
		TOKEN_BRACKET_CLOSE, //)
		TOKEN_SQUARE_OPEN,	 //[
		TOKEN_SQUARE_CLOSE,	 //]

		TOKEN_SYMBOL,
	};

	struct Token
	{
		String text;
		ETokenCode code;
		uint32 line;

		Token() : line(1), code(TOKEN_EMPTY) {}

		Token(const String& _text, ETokenCode _code, uint32 _line) :
			text(_text),
			code(_code),
			line(_line)
		{}

		operator String() const { return text; }

		bool operator==(const String& str) { return text == str; }
		bool operator!=(const String& str) { return !this->operator==(str); }

		static const char* typeToString(uint32 code)
		{
			static const char* table[] =
			{
				"EMPTY",
				"IDENTIFIER",
				"INTEGER",
				"FLOAT",
				"STRING",

				//keywords
				"struct", "cbuffer", "register",

				";", ":", ".", "=", ",", "#",

				"{", "}", "(", ")", "[", "]"

				"SYMBOL"
			};

			return table[code];
		}
	};
}

/*
namespace std
{
	std::ostream& operator<<(std::ostream& out, const ts::Token& tok)
	{
		out << tok.text;
		return out;
	}
}
*/
