/*
	Shader Definition file importer
	
	source
*/

#include "importer.h"

#include <tscore/filesystem/pathhelpers.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

using namespace ts;
using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct SShaderEntry
{
	StaticString<128> name;
	SShaderInfo info;

	bool operator==(const SShaderEntry& rhs)
	{
		return this->name == rhs.name;
	}
};

bool isSpace(char c)
{
	return ((c == ' ') || (c == '\n') || (c == '\t'));
}

bool isLetter(char c)
{
	return ((c <= 'z') && (c >= 'a')) || ((c <= 'Z') && (c <= 'A'));
}

bool isNumber(char c)
{
	return ((c <= '9') && (c >= '0'));
}

//If char marks the start of a new token
bool isEscape(char c)
{
	return isSpace(c) || (c == 0) || (c == ';') || (c == '\"');
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Internal implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct CShaderDefImporter::Impl
{
	vector<SShaderEntry> shadersEntries;

	//Parses shader definition data from stream and stores result in shaderEntries
	Impl(istream& stream)
	{
		vector<string> tokens;

		tokenize(stream, tokens);

		for (auto t : tokens)
		{
			cout << "\"" << t << "\"\n";
		}
	}

private:
	
	void tokenize(istream& stream, vector<string>& tokens)
	{
		//States
		string stateToken;
		bool stateInStr = false;
		char statePrevChar = 0;

		//Tokenize the string

		//Read from stream while not EOF
		char ch = 0;
		while (stream.read(&ch, 1))
		{
			if (ch == '\"')
			{
				stateInStr = !stateInStr;

				//Enquote
				if (!stateInStr)
				{
					stateToken += ch;
				}

				//Cache token
				if (stateToken.size() > 0)
				{
					tokens.push_back(stateToken);
					stateToken.clear();
				}

				//Quote
				if (stateInStr)
				{
					stateToken += ch;
				}
			}
			else
			{
				//If the current sequence of chars we are parsing lies between quotes, do not process
				if (stateInStr)
				{
					stateToken += ch;
				}
				else
				{
					//Convert all whitespace to space
					if (isSpace(ch))
					{
						ch = ' ';
					}

					//Don't write unecessary whitespace
					if (!(isSpace(ch) && isSpace(statePrevChar)))
					{
						//Don't actually write out any spaces
						if (ch != ' ')
						{
							stateToken += ch;
						}

						//If current char is a split char
						//and previous char is an escape char
						//then cache and reset current token
						if (((ch == ';') || (ch == '}') || (ch == '{')) || (ch == ' '))
						{
							if (stateToken.size() > 0)
							{
								tokens.push_back(stateToken);
								stateToken.clear();
							}
						}
					}
				}
			}

			statePrevChar = ch;
		}
	}

	int parse(const vector<string>& tokens)
	{
		for (const string& token : tokens)
		{
			char c = token[0];

			if (isLetter(c))
			{
				//Is identifier
			}
			else if (isNumber(c))
			{
				//Is number value
			}
			else if (c == '\"')
			{
				//Is string value
			}
			else if (c == ';')
			{

			}
			else if (c == '{')
			{

			}
			else if (c == '}')
			{

			}
		}
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Ctor/dtor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CShaderDefImporter::CShaderDefImporter(const char* data, size_t datalen)
{
	loadData(data, datalen);
}

CShaderDefImporter::CShaderDefImporter(const std::string& data)
{
	loadData(data);
}

CShaderDefImporter::CShaderDefImporter(const ts::Path& path)
{
	load(path);
}

CShaderDefImporter::~CShaderDefImporter()
{
	if (pImpl)
	{
		delete pImpl;
		pImpl = nullptr;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CShaderDefImporter::loadData(const char* data, size_t datalen)
{
	if (pImpl)
	{
		delete pImpl;
	}

	stringstream stream;
	stream.write(data, datalen);

	pImpl = new Impl(stream);

	return true;
}

bool CShaderDefImporter::loadData(const string& data)
{
	return this->loadData(data.c_str(), data.size());
}

bool CShaderDefImporter::load(const Path& path)
{
	if (pImpl)
	{
		delete pImpl;
	}

	ifstream file(path.str());
	pImpl = new Impl(file);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32 CShaderDefImporter::getShaderCount() const
{
	return (uint32)pImpl->shadersEntries.size();
}

void CShaderDefImporter::getShaderInfo(uint32 idx, SShaderInfo& info) const
{
	if (idx < pImpl->shadersEntries.size())
	{
		info = pImpl->shadersEntries.at(idx).info;
	}
}

void CShaderDefImporter::findShaderInfo(const char* shaderName, SShaderInfo& info) const
{
	SShaderEntry e;
	e.name.set(shaderName);

	auto it = find(pImpl->shadersEntries.begin(), pImpl->shadersEntries.end(), e);

	if (it != pImpl->shadersEntries.end())
	{
		info = it->info;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
