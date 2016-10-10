/*
	Path helper function source
*/

#include "pathhelpers.h"

#include <Windows.h>
#include <Shlwapi.h>
#include <fstream>

using namespace std;

#pragma comment(lib, "Shlwapi.lib")

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	bool isAbsolutePath(const Path& path)
	{
		return (::PathIsRelativeA(path.str()) == FALSE);
	}

	bool isFile(const Path& path)
	{
		return ifstream(path.str()).good();
	}

	bool isDirectory(const Path& path)
	{
		return (::GetFileAttributesA(path.str()) == FILE_ATTRIBUTE_DIRECTORY);
	}

	bool resolveFile(const Path& inpath, Path& foundpath, const Path* searchPathArray, size_t searchPathArraySize)
	{
		if (searchPathArray && searchPathArraySize)
		{
			//Iterate over each search directory
			for (size_t i = 0; i < searchPathArraySize; i++)
			{
				Path p(searchPathArray[i]);
				p.addDirectories(inpath);

				if (isFile(p))
				{
					foundpath = p;
					return true;
				}
			}
		}

		return false;
	}

	bool searchFile(const Path& inpath, Path& foundpath, const Path* searchPathArray, size_t searchPathArraySize)
	{
		return false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
