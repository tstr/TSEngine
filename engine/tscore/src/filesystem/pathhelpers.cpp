/*
	Path helper function source
*/

#include <tscore/filesystem/pathhelpers.h>

#include <Windows.h>
#include <Shlwapi.h>
#include <Shlobj.h>

#include <fstream>

using namespace std;

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Shell32.lib")

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

	fstream createFile(const Path& name, int flags)
	{
		if (!isDirectory(name))
		{
			string sbuf(name.getParent().str());
			for (char& c : sbuf)
			{
				if (c == '/')
				{
					c = '\\';
				}
			}

			int err = SHCreateDirectoryExA(nullptr, sbuf.c_str(), nullptr);

			if (err == ERROR_ALREADY_EXISTS || err == ERROR_SUCCESS)
			{
				return fstream(name.str(), flags);
			}
		}

		return fstream();
	}

	bool findPaths(const Path& path, std::vector<Path>& paths)
	{
		WIN32_FIND_DATA findInfo;

		HANDLE hFind = FindFirstFileA(path.str(), &findInfo);

		if (INVALID_HANDLE_VALUE == hFind)
		{
			return false;
		}

		do
		{
			paths.push_back(findInfo.cFileName);

		} while (FindNextFileA(hFind, &findInfo) != 0);

		if (GetLastError() != ERROR_NO_MORE_FILES)
		{
			return false;
		}

		FindClose(hFind);

		return true;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
