/*
	Command line arguments
*/

#include <cli/Arguments.h>

#include <iostream>
#include <iomanip>

using namespace std;
using namespace ts;
using namespace ts::cli;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Helper functions
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// token either has "-" or "--" in front of it
static bool isParameterTag(const String& token)
{
	return !token.empty() && ((token.front() == '-') || (token.compare(0, 2, "--") == 0));
}

static String formatParameterTag(const String& name)
{
	return (isParameterTag(name)) ? name : (String("--") + name);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Set parameter handlers
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ArgumentReader::addParameter(const String& name, String& value, const String& desc)
{
	//Set parameter callback
	m_parameterMap[formatParameterTag(name)] = Parameter(
		[&value](String& s) {
		value = s;
	}, 1, desc);
}

void ArgumentReader::addOption(const String& name, bool& present, const String& desc)
{
	m_parameterMap[formatParameterTag(name)] = Parameter(
		[&present](String& s) {
		present = true;
	}, 0, desc);
}

void ArgumentReader::setUnused(ParameterList& list)
{
	m_parameterMap[""] = Parameter(
		[&list](String& value) {
		list.push_back(value);
	}, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Parse method
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ArgumentReader::parse(int argc, char** argv)
{
	//For each argument
	for (int i = 1; i < argc; i++)
	{
		String arg = argv[i];

		if (isParameterTag(arg))
		{
			auto it = m_parameterMap.find(arg);
			
			if (it == m_parameterMap.end())
			{
				cerr << "ERROR: Unknown command line option \"" << arg << "\"\n";
				return UNKNOWN_PARAMETER_TAG;
			}
			else
			{
				const Parameter& param = it->second;

				//If parameter requires extra arguments
				if (param.hasArgs)
				{
					//Next argument in list becomes input for parameter callback
					i++;
					arg = argv[i];

					if (isParameterTag(arg))
					{
						cerr << "ERROR: Unexpected parameter tag \"" << arg << "\", expected string value\n";
						return MISSING_PARAMETER_VALUE;
					}

					it->second.callback(arg);
				}
				//Otherwise just pass an empty string
				else
				{
					it->second.callback(String());
				}
			}
		}
		else
		{
			//If current argument is not a parameter tag then
			//Store this argument in the unused list
			auto it = m_parameterMap.find("");
			if (it != m_parameterMap.end())
			{
				//Callback appends argument to the unused list
				it->second.callback(arg);
			}
		}
	}

	return SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Debug print 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ArgumentReader::print(ostream& stream)
{
	stream << "Parameter list:\n";

	for (const auto& param : m_parameterMap)
	{
		//Ignore the unused parameter because it does not have a name
		if (param.first != "")
		{
			stream << setw(25) << left;

			if (param.second.hasArgs)
			{
				//Print parameter name
				stream << format("% <argument>", param.first);
			}
			else
			{
				stream << param.first;
			}

			//If a parameter description has been specified
			if (param.second.desc.size() > 0)
			{
				stream << setw(0) << internal;
				stream << ": ";
				stream << param.second.desc;
			}

			stream << endl;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
