/*
	Shader Parser class source
*/

#include "ShaderParser.h"

#include "util/Scanner.h"

#include <iostream>

using namespace ts;
using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderParser::isBasicType(const String& identifier) const
{
	if (const MemberType* t = m_types->type(identifier))
		return t->isBasic();

	return false;
}

bool ShaderParser::isStructType(const String& identifier) const
{
	if (const MemberType* t = m_types->type(identifier))
		return t->isComposite();

	return false;
}

bool ShaderParser::isType(const String& identifier) const
{
	return m_types->isType(identifier);
}

bool ShaderParser::isResourceType(const String& identifier) const
{
	const static set<string> resourceTypes = {
		"Texture1D", "Texture2D", "Texture3D", "TextureCube", "SamplerState"
	};

	return resourceTypes.find(identifier) != resourceTypes.end();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderParser::isFunction(const String& name) const
{
	return m_functions.find(FunctionDeclaration(name)) != m_functions.end();
}

ShaderParser::FunctionDeclaration ShaderParser::getFunction(const String& name) const
{
	auto it = m_functions.find(FunctionDeclaration(name));

	if (it == m_functions.end())
	{
		return *it;
	}
	else
	{
		return FunctionDeclaration();
	}
}


bool ShaderParser::isResource(const String& name) const
{
	return m_resources.find(ResourceDeclaration(name)) != m_resources.end();

}

ShaderParser::ResourceDeclaration ShaderParser::getResource(const String& name) const
{
	auto it = m_resources.find(ResourceDeclaration(name));

	if (it == m_resources.end())
	{
		return *it;
	}
	else
	{
		return ResourceDeclaration();
	}
}

bool ShaderParser::isConstantBuffer(const String& name) const
{
	return m_constants.find(ConstantsDeclaration(name)) != m_constants.end();

}

ShaderParser::ConstantsDeclaration ShaderParser::getConstantBuffer(const String& name) const
{
	auto it = m_constants.find(ConstantsDeclaration(name));

	if (it == m_constants.end())
	{
		return *it;
	}
	else
	{
		return ConstantsDeclaration();
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderParser::Register ShaderParser::parseRegister(Scanner& scan, const String& prefixes)
{
	scan.tryNext(TOKEN_REGISTER);
	scan.tryNext(TOKEN_BRACKET_OPEN);

	Token t = scan.next();
	Register reg;

	reg.prefix = t.text.front();

	if (prefixes.find(reg.prefix) != String::npos)
	{
		string toffset = (t.text.c_str() + 1);
		reg.slot = stoi(toffset);
	}
	else
	{
		reg.prefix = 0;
		return reg;
	}

	scan.tryNext(TOKEN_BRACKET_CLOSE);

	return reg;
}

void ShaderParser::parseFunctionParameters(Scanner& scan, std::vector<StructMember>& parameters)
{
	// (
	scan.tryNext(TOKEN_BRACKET_OPEN);

	while (!scan.isNext(TOKEN_BRACKET_CLOSE))
	{
		StructMemberInfo paramInfo;
		paramInfo.arraySize = 1;

		//Parameter type
		if (m_types->isType(scan.tryNext(TOKEN_IDENTIFIER)))
		{
			paramInfo.typeName = scan.current();
		}
		else
		{
			cerr << "Invalid type name \"" << scan.current().text << "\"\n";
			throw ParserException("Invalid type");
		}

		//Parameter name
		paramInfo.name = scan.tryNext(TOKEN_IDENTIFIER);

		//Semantic
		if (scan.isNext(TOKEN_COLON))
		{
			scan.next();
			paramInfo.semantic = scan.tryNext(TOKEN_IDENTIFIER);
		}

		if (scan.isNext(TOKEN_COMMA))
		{
			scan.next();
		}

		parameters.push_back(StructMember(m_types.get(), paramInfo));
	}

	// )
	scan.tryNext(TOKEN_BRACKET_CLOSE);
}

void ShaderParser::parseFunctionDeclaration(Scanner& scan)
{
	//<type> <identifier> ( [<typeA> <param0>, <typeB> <param1>, ...] ) [: <semantic>] { ... }

	FunctionDeclaration func;

	func.returnType = m_types->type(scan.tryNext(TOKEN_IDENTIFIER));

	//Verify return type exists
	if (func.returnType == nullptr)
	{
		cerr << "Invalid type name \"" << scan.current().text << "\"\n";
		throw ParserException("Parser exception");
	}

	func.name = scan.tryNext(TOKEN_IDENTIFIER);

	parseFunctionParameters(scan, func.parameters);

	//Has return type semantic
	if (scan.isNext(TOKEN_COLON))
	{
		scan.next();
		func.returnSemantic = scan.tryNext(TOKEN_IDENTIFIER);
	}

	//Skip function body
	if (scan.next() == "{")
	{
		int counter = 1;

		while (counter > 0)
		{
			Token t = scan.next();

			if (t == "{")
				counter++;

			if (t == "}")
				counter--;
		}
	}

	m_functions.insert(func);
}

void ShaderParser::parseResourceDeclaration(Scanner& scan)
{
	ResourceDeclaration res;

	//Resource type
	res.type = scan.next();

	//Resource name
	res.name = scan.tryNext(TOKEN_IDENTIFIER);

	//Register
	if (scan.isNext(TOKEN_COLON))
	{
		scan.next();
		res.reg = parseRegister(scan, "tsu");
	}

	scan.tryNext(TOKEN_SEMICOLON);

	m_resources.insert(res);
}

void ShaderParser::parseConstantsDeclaration(Scanner& scan)
{
	scan.tryNext(TOKEN_CBUFFER);
	{
		//struct <identifier>
		ConstantsDeclaration constants;
		constants.name = scan.tryNext(TOKEN_IDENTIFIER).text;

		// : register(b0)
		if (scan.isNext(TOKEN_COLON))
		{
			scan.next();
			constants.reg = parseRegister(scan, "tb");
		}

		// {
		scan.tryNext(TOKEN_BLOCK_OPEN);
		{
			// }
			while (!scan.isNext(TOKEN_BLOCK_CLOSE))
			{
				StructMemberInfo member;
				parseStructMember(scan, member);

				StructMember constant(m_types.get(), member);

				if (constant.type() == nullptr)
				{
					cerr << "Invalid type name \"" << member.typeName << "\"\n";
					throw ParserException("Parser exception");
				}

				constants.members.push_back(constant);
			}

			// skip }
			scan.next();

			// ;
			scan.tryNext(TOKEN_SEMICOLON);
		}

		m_constants.insert(constants);
	}
}

void ShaderParser::parseStructMember(Scanner& scan, StructMemberInfo& member)
{
	//Type
	member.typeName = scan.tryNext(TOKEN_IDENTIFIER);
	member.arraySize = 1;
	{
		//Name
		member.name = scan.tryNext(TOKEN_IDENTIFIER);
		{
			//Array - [<integer>]
			if (scan.isNext(TOKEN_SQUARE_OPEN))
			{
				scan.next();

				member.arraySize = stoi(scan.tryNext(TOKEN_INTEGER));

				scan.tryNext(TOKEN_SQUARE_CLOSE);
			}

			//Optional semantic
			if (scan.isNext(TOKEN_COLON))
			{
				scan.next();

				//Semantic
				member.semantic = scan.tryNext(TOKEN_IDENTIFIER);
			}

			//End statement
			scan.tryNext(TOKEN_SEMICOLON);
		}
	}
}

void ShaderParser::parseStruct(Scanner& scan)
{
	scan.tryNext(TOKEN_STRUCT);
	{
		//struct <identifier>
		string structName = scan.tryNext(TOKEN_IDENTIFIER).text;
		vector<StructMemberInfo> members;

		// {
		scan.tryNext(TOKEN_BLOCK_OPEN);
		{
			// }
			while (!scan.isNext(TOKEN_BLOCK_CLOSE))
			{
				StructMemberInfo member;
				parseStructMember(scan, member);
				members.push_back(member);
			}

			// skip }
			scan.next();

			// ;
			scan.tryNext(TOKEN_SEMICOLON);
		}

		if (!m_types->defineType(structName, members))
		{
			cerr << "Invalid type name \"" << structName << "\"\n";
			throw ParserException("Parser exception");
		}
	}
}


bool ShaderParser::parse(istream& stream)
{
	//Reset state
	m_state = true;
	m_types.reset(new TypeContext());
	m_functions.clear();
	m_resources.clear();
	m_constants.clear();

	Scanner scan(stream);

	try
	{
		while (scan.hasNext())
		{
			Token tok = scan.peek();

			switch (tok.code)
			{
			case TOKEN_STRUCT:
			{
				parseStruct(scan);
				break;
			}
			case TOKEN_CBUFFER:
			{
				parseConstantsDeclaration(scan);
				break;
			}
			case TOKEN_IDENTIFIER:
			{
				if (isResourceType(tok.text))
				{
					parseResourceDeclaration(scan);
				}
				else if ((tok.text == "void") || isType(tok.text))
				{
					parseFunctionDeclaration(scan);
				}
				else
				{
					//Skip token
					scan.next();
				}

				break;
			}
			default:
			{
				//Skip token
				scan.next();
			}
			}
		}
	}
	catch (ParserException& e)
	{
		//Log error
		//cerr << e.what() << endl;
		m_state = false;
	}

	return m_state;
}
