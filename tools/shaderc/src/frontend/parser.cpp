/*
	Shader Info Parser

	todo: macros
	
	source
*/

#include "parser.h"

#include <tscore/pathutil.h>

#include <cassert>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <stack>

#include "tree.h"

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
	return ((c <= 'z') && (c >= 'a')) || ((c <= 'Z') && (c >= 'A'));
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
struct CShaderInfoParser::Impl
{
private:

	class Block
	{
	private:

		/*
			[Block]
			[Id0][Id1] ... [IdN] {

				[Stat0];
				[Stat1];
				[Stat2];
				...
				[StatN];

				[Block]
				[Id0][Id1] {
					...
				}
			}

		*/

		vector<string> m_tokens;

	public:

		void pushToken(const string& tok)
		{
			m_tokens.push_back(tok);
		}

		const vector<string>& getTokens() const
		{
			return m_tokens;
		}

		void clearTokens()
		{
			m_tokens.clear();
		}
	};

public:

	vector<SShaderEntry> shaderEntries;

	//Parses shader definition data from stream and stores result in shaderEntries
	Impl(istream& stream)
	{
		vector<string> tokens;
		Tree<Block> blocks;
		Tree<Block>::NodeId root;

		tokenize(stream, tokens);

		if (int err = buildTree(tokens, blocks, root))
		{
			cerr << "Error building statement tree : " << err << endl;
		}

		/*
		int depth = -1;
		printTree(blocks, root, depth);
		*/

		if (int err = parseEntries(blocks, root))
		{
			cerr << "Error parsing statement tree : " << err << endl;
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

				//Endquote
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

	void printTree(const Tree<Block>& blocks, Tree<Block>::NodeId node, int& depth)
	{
		Block b;
		blocks.getNodeValue(node, b);

		for (int i = 0; i < depth; i++)
			cout << "\t";

		for (const string& tok : b.getTokens())
		{
			cout << "[" << tok << "]";
		}

		cout << "\n";

		depth++;

		for (const auto& child : blocks.getChildren(node))
		{
			printTree(blocks, child, depth);
		}

		depth--;
	}

	int buildTree(const vector<string>& tokens, Tree<Block>& blocks, Tree<Block>::NodeId& root)
	{
		typedef Tree<Block>::NodeId Node;

		root = blocks.allocNode(Block());

		//Initialize root node
		stack<Node> stateNodeStack;
		stateNodeStack.push(root);
		Block stateBlock;

		for (auto tok = tokens.begin(); tok != tokens.end(); tok++)
		{
			char ch = tok->front();

			if (ch == ';')
			{
				//End of statement
				Node curNode = blocks.allocNode(stateBlock);
				blocks.linkNode(stateNodeStack.top(), curNode);
				stateBlock.clearTokens();
			}
			else if (ch == '{')
			{
				//Start of new block
				Node curNode = blocks.allocNode(stateBlock);
				blocks.linkNode(stateNodeStack.top(), curNode);
				stateBlock.clearTokens();

				stateNodeStack.push(curNode);
			}
			else if (ch == '}')
			{
				//End of block
				stateNodeStack.pop();
			}
			else
			{
				stateBlock.pushToken(*tok);
			}
		}

		stateNodeStack.pop();

		return (int)stateNodeStack.size();
	}


	void parseShaderStage(const Tree<Block>& blockTree, Tree<Block>::NodeId stageNode, SShaderStageInfo& info)
	{
		typedef Tree<Block>::NodeId Node;

		//Parse statements
		for (uint32 i = 0; i < blockTree.getChildCount(stageNode); i++)
		{
			Block statBlock;
			Node statNode = blockTree.getChild(stageNode, i);
			blockTree.getNodeValue(statNode, statBlock);

			const auto& statTokens = statBlock.getTokens();

			if (statTokens[0] == "file")
			{
				//[name][=][value]
				assert(statTokens.size() == 3);											   //Statement must have 3 tokens
				assert(statTokens[1] == "=");											   //Second token must be an assignment operator
				assert((statTokens[2].front() == '\"') && (statTokens[2].back() == '\"')); //Third token must be a string value
				
				//Trim quotes from string
				info.sourceFile = statTokens[2].substr(1, statTokens[2].size() - 2);

			}
			else if (statTokens[0] == "entrypoint")
			{
				//[name][=][value]
				assert(statTokens.size() == 3);												//Statement must have 3 tokens
				assert(statTokens[1] == "=");												//Second token must be an assignment operator
				assert((statTokens[2].front() == '\"') && (statTokens[2].back() == '\"'));  //Third token must be a string value

				//Trim quotes from string
				info.entryPoint = statTokens[2].substr(1, statTokens[2].size() - 2);
			}
			else if (statTokens[0] == "macros")
			{
				//todo
			}
		}
	}

	int parseEntryBlock(const Tree<Block>& blockTree, Tree<Block>::NodeId root, SShaderEntry& entry)
	{
		typedef Tree<Block>::NodeId Node;

		//Shader declaration block
		Block decl;
		blockTree.getNodeValue(root, decl);

		if (decl.getTokens().size() == 2)
		{
			//[shader][name]{
			entry.name = decl.getTokens()[1];
		}
		else
		{
			cerr << "Shader name expected\n";
			return 1;
		}

		//Iterate over each stage block
		for (uint32 i = 0; i < blockTree.getChildCount(root); i++)
		{
			Node stageNode = blockTree.getChild(root, i);

			Block block;
			blockTree.getNodeValue(stageNode, block);
			
			const auto& tokens = block.getTokens();

			if (!tokens.empty() && (tokens[0] == "stage") && (tokens.size() == 2))
			{
				if (tokens[1] == "pixel")
				{
					parseShaderStage(blockTree, stageNode, entry.info.pixelStage);
				}
				else if (tokens[1] == "vertex")
				{
					parseShaderStage(blockTree, stageNode, entry.info.vertexStage);
				}
				else if (tokens[1] == "geometry")
				{
					parseShaderStage(blockTree, stageNode, entry.info.geometryStage);
				}
				else if (tokens[1] == "domain")
				{
					parseShaderStage(blockTree, stageNode, entry.info.domainStage);
				}
				else if (tokens[1] == "hull")
				{
					parseShaderStage(blockTree, stageNode, entry.info.hullStage);
				}
			}
		}


		cout << endl;

		return 0;
	}

	int parseEntries(const Tree<Block>& blockTree, Tree<Block>::NodeId root)
	{
		shaderEntries.clear();

		typedef Tree<Block>::NodeId Node;

		//Iterate over top level blocks
		for (uint32 i = 0; i < blockTree.getChildCount(root); i++)
		{
			Node node = blockTree.getChild(root, i);

			Block block;
			blockTree.getNodeValue(node, block);

			const auto& tokens = block.getTokens();

			if (!tokens.empty() && tokens[0] == "shader")
			{
				SShaderEntry entry;
				if (int err = parseEntryBlock(blockTree, node, entry))
				{
					return err;
				}

				shaderEntries.push_back(entry);
			}
		}

		return 0;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Ctor/dtor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CShaderInfoParser::CShaderInfoParser(const char* data, size_t datalen)
{
	loadData(data, datalen);
}

CShaderInfoParser::CShaderInfoParser(const std::string& data)
{
	loadData(data);
}

CShaderInfoParser::CShaderInfoParser(const ts::Path& path)
{
	load(path);
}

CShaderInfoParser::~CShaderInfoParser()
{
	if (pImpl)
	{
		delete pImpl;
		pImpl = nullptr;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CShaderInfoParser::loadData(const char* data, size_t datalen)
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

bool CShaderInfoParser::loadData(const string& data)
{
	return this->loadData(data.c_str(), data.size());
}

bool CShaderInfoParser::load(const Path& path)
{
	if (pImpl)
	{
		delete pImpl;
	}

	ifstream file(path.str());

	if (file.good())
	{
		pImpl = new Impl(file);
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32 CShaderInfoParser::getShaderCount() const
{
	return (uint32)pImpl->shaderEntries.size();
}

void CShaderInfoParser::getShaderInfo(uint32 idx, string& shaderName, SShaderInfo& info) const
{
	if (idx < pImpl->shaderEntries.size())
	{
		const auto& entry = pImpl->shaderEntries.at(idx);
		info = entry.info;
		shaderName = entry.name.str();
	}
}

void CShaderInfoParser::findShaderInfo(const string& shaderName, SShaderInfo& info) const
{
	SShaderEntry e;
	e.name.set(shaderName);

	auto it = find(pImpl->shaderEntries.begin(), pImpl->shaderEntries.end(), e);

	if (it != pImpl->shaderEntries.end())
	{
		info = it->info;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
