/*
	CLI Argument Reader class

	Reads a sequence of command line arguments and parses a set of parameters from them.

	terminology:
	- Argument  = an individual token.
	- Parameter = a set of arguments, composed of a tag and optionally a value
	- Option    = a parameter with no value argument

	usage:

		bool enableSomething;
		String outputDirectory = "./";
		CommandLineArguments::ParameterList list;

		CommandLineArguments args;
		args.addOption("enable", enableSomething);		//If a parameter called "--enable" is found, enableSomething is set to true
		args.addParameter("output", outputDirectory);	//If a parameter called "--output" is found, outputDirectory is set to the subsequent argument
		args.setUnused(list);							//Unused parameters go here ie. arguments without a corresponding parameter tag
		args.parse(argc, argv);
*/

#pragma once

#include <tscore/strings.h>

#include <map>
#include <functional>

namespace ts
{
	namespace cli
	{
		class ArgumentReader
		{
		public:

			enum EErrorCode
			{
				SUCCESS,
				UNKNOWN_PARAMETER_TAG,
				MISSING_PARAMETER_VALUE,
			};
			
			typedef std::vector<String> ParameterList;

			//Add parameters and options
			void addParameter(const String& name, String& value);
			void addOption(const String& name, bool& present);
			void setUnused(ParameterList& list);
			
			//Parse c-style CLI arguments
			int parse(int argc, char** argv);

			//Debug print list of parameters and options
			void print(std::ostream& stream);

		private:

			using ParameterCallback = std::function<void(String&)>;
			
			// Internal parameter structure
			struct Parameter
			{
				bool hasArgs;
				ParameterCallback callback;

				Parameter() :
					callback([](String&) {}),
					hasArgs(0)
				{}

				Parameter(ParameterCallback callback, bool has = false) :
					callback(callback),
					hasArgs(has)
				{}
			};
			
			using ParameterMap = std::map<String, Parameter>;

			ParameterMap m_parameterMap;
		};
	}
}
