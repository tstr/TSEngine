/*
	CLI util constants
*/

#pragma once

namespace ts
{
	namespace cli
	{
		/*
			Common exit codes
		*/
		enum EExitCodes
		{
			CLI_EXIT_SUCCESS			= 0,
			CLI_EXIT_INVALID_ARGUMENT	= 1,
			CLI_EXIT_FAILURE			= -1,
		};
	}
}
