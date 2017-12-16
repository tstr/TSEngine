/*
	Lexical Scanner class:
	
	Splits a sequence of characters into a sequence of tokens which can be read.
*/

#pragma once

#include <tscore/strings.h>

#include "Token.h"

//////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	class Scanner
	{
	private:

		//Character sequence
		String m_seq;
		//Character sequence read pointer
		String::iterator m_seqReader;

		uint32 m_lineCounter;
		Token m_curToken;

		//Test if there is another char in the sequence
		bool hasChar()
		{
			return peekChar() != 0;
		}

		//Returns next char in sequence and increments read pointer
		char getChar()
		{
			if (m_seqReader == m_seq.end())
			{
				return 0;
			}
			else
			{
				//Postfix increment
				return *(m_seqReader++);
			}
		}

		//Return next char in sequence without incrementing read pointer
		char peekChar()
		{
			if (m_seqReader == m_seq.end())
			{
				return 0;
			}
			else
			{
				//No increment
				return *m_seqReader;
			}
		}

	public:

		Scanner(std::istream& in) :
			m_lineCounter(1)
		{
			using namespace std;
		
			//Read entire stream into string
			stringstream stream;
			stream << in.rdbuf();
			m_seq = move(stream.str());
			m_seqReader = m_seq.begin();
		}

		bool hasNext();

		Token next();
		Token peek();
		Token current();
	
		Token tryNext(uint32 code);
		bool nextIf(uint32 code);
		bool isNext(uint32 code);

		void reset();
	};
	
	class ParserException : public std::exception
	{
	public:

		ParserException(const char* str) : exception(str)
		{}

		ParserException(const String& str) : exception(str.c_str())
		{}
	};
}
//////////////////////////////////////////////////////////////////////////////////////////////////
