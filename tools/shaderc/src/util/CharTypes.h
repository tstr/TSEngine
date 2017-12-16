/*
	Char types header, for classifying character types
*/

#pragma once

#include <string.h>
#include <tscore/types.h>

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
}