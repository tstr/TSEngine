/*
	Lexical Scanner class source
*/

#include "Scanner.h"

#include <iostream>

using namespace std;
using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Scanner::reset()
{
	m_seqReader = m_seq.begin();
	m_lineCounter = 1;
}

Token Scanner::current()
{
	return m_curToken;
}

Token Scanner::peek()
{
	auto save_reader = m_seqReader;
	Token save_cur = m_curToken;
	uint32 save_line = m_lineCounter;

	Token t = this->next();

	m_seqReader = save_reader;
	m_curToken = save_cur;
	m_lineCounter = save_line;

	return t;
}

bool Scanner::hasNext()
{
	return this->peek() != "";
}

Token Scanner::next()
{
	Token& token = m_curToken;
	token.line = m_lineCounter;
	token.text.clear();
	token.code = TOKEN_EMPTY;

	while(hasChar())
	{
		switch (CharTypes::get(peekChar()))
		{
			case CHAR_CLASS_WHITESPACE:
			{
				if (getChar() == '\n')
				{
					token.line = ++m_lineCounter;
				}

				break;
			}
			case CHAR_CLASS_DIGIT:
			{
				token.code = TOKEN_INTEGER;

				//Matches integer sequence
				while (CharTypes::get(peekChar()) == CHAR_CLASS_DIGIT)
				{
					token.text += getChar();
				}

				//Matches float sequence
				if (peekChar() == '.')
				{
					token.code = TOKEN_FLOAT;
					token.text += getChar();

					while (CharTypes::get(peekChar()) == CHAR_CLASS_DIGIT)
					{
						token.text += getChar();
					}

					if (peekChar() == 'f')
					{
						token.text += getChar();
					}
				}

				return token;
			}
			case CHAR_CLASS_LETTER:
			{
				//Expect identifier
				token.code = TOKEN_IDENTIFIER;

				while (
					(CharTypes::get(peekChar()) == CHAR_CLASS_LETTER) ||
					(CharTypes::get(peekChar()) == CHAR_CLASS_DIGIT) ||
					(peekChar() == '_')
					)
				{
					token.text += getChar();
				}

				//Keywords
				if (token.text == "struct")
				{
					token.code = TOKEN_STRUCT;
				}
				else if ((token.text == "cbuffer") || (token.text == "tbuffer"))
				{
					token.code = TOKEN_CBUFFER;
				}
				else if (token.text == "register")
				{
					token.code = TOKEN_REGISTER;
				}

				return token;
			}
			default:
			{
				//Expect string
				if (peekChar() == '\"')
				{
					token.code = TOKEN_STRING;
					token.text += getChar();

					while (peekChar() != '\"')
					{
						char c = getChar();

						token.text += c;

						//If current char marks an escape sequence
						//And the next char is a quote then include nested string
						if ((c == '\\') && (peekChar() == '\"'))
						{
							token.text += getChar();
						}
					}
					//Append extra quote mark
					token.text += getChar();
				}
				//If char starts with underscore then expect identifier
				else if (peekChar() == '_')
				{
					token.code = TOKEN_IDENTIFIER;
					token.text += getChar();
					break;
				}
				else
				{
					char c = getChar();
					token.code = TOKEN_SYMBOL;
					token.text = c;

					switch (c)
					{
					case ';': token.code = TOKEN_SEMICOLON; break;
					case ':': token.code = TOKEN_COLON; break;
					case '.': token.code = TOKEN_DOT; break;
					case '=': token.code = TOKEN_EQUALS; break;
					case ',': token.code = TOKEN_COMMA; break;
					case '#': token.code = TOKEN_HASH; break;

					case '{': token.code = TOKEN_BLOCK_OPEN; break;
					case '}': token.code = TOKEN_BLOCK_CLOSE; break;
					case '(': token.code = TOKEN_BRACKET_OPEN; break;
					case ')': token.code = TOKEN_BRACKET_CLOSE; break;
					case '[': token.code = TOKEN_SQUARE_OPEN; break;
					case ']': token.code = TOKEN_SQUARE_CLOSE; break;
					}
				}

				return token;
			}
		}
	}

	return token;
}

Token Scanner::tryNext(uint32 code)
{
	Token t = this->next();

	if (t.code == code)
	{
		return t;
	}

	cerr << "Unexpected token \"" << t.text << "\" expected \"" << Token::typeToString(code) << "\"\n";

	throw ParserException(t.text.c_str());
}

bool Scanner::isNext(uint32 code)
{
	return this->peek().code == code;
}

bool Scanner::nextIf(uint32 code)
{
	if (isNext(code))
	{
		next();
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
