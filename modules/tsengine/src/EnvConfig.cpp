/*
	Config file loading source

	Loads an INI config file and stores all entries in the Variable Table
*/

#include <tsengine/Env.h>
#include <tscore/debug/log.h>

#include "IniReader.h"

using namespace ts;
using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void EngineEnv::initConfig(const Path& cfgpath)
{
	m_vars.reset(new VarTable());
	
	INIReader config(cfgpath);

	INIReader::SectionArray secs;
	config.getSections(secs);

	for (auto s : secs)
	{
		INIReader::PropertyArray props;
		config.getSectionProperties(s, props);

		for (auto p : props)
		{
			m_vars->set(format("%.%", s, p.key), p.value);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
