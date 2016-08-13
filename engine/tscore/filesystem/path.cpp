/*
	Path class source

	todo:

	- filter special characters
	- iterators
*/

#include "path.h"

#define NOMINMAX
#include <Windows.h>

#include <algorithm>

using namespace ts;
using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Path::composePath(const char* pathstr)
{
	StaticString<Path::MaxLength> strbuf(pathstr);

	//Iterate over path and replace all instances of '\\' with '/' as well as find the length
	uint32 length = 0;
	for (uint32 i = 0; i < Path::MaxLength; i++)
	{
		char c = strbuf.at(i);

		if (c == 0)
		{
			length = i;
			break;
		}

		if (c == '\\')
		{
			strbuf[i] = '/';
		}
	}

	//The first and last characters in the path must not be path splitters

	ptrdiff offset = 0;
	if (strbuf[0] == '\\' || strbuf[0] == '/')
	{
		//If the first character is a path splitter then copy strbuf with an offset of one
		//to ensure that the first character isn't copied
		offset = 1;
	}

	if (strbuf[length - 1] == '\\' || strbuf[length - 1] == '/')
	{
		strbuf[length - 1] = 0;
	}

	m_path.set(strbuf.get() + offset);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint16 Path::getDirectoryCount() const
{
	uint16 count = 0;

	for (uint i = 0; i < Path::MaxLength; i++)
	{
		//exit early if we reach the null character
		if (m_path[i] == 0)
			break;
		else if (m_path[i] == '/')
			count++;
	}

	//Increase by one to take into account the last directory
	count++;

	return count;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

Path Path::getDirectory(uint16 idx) const
{
	uint16 idxcounter = 0;	//Current directory index
	uint16 prevchar = 0;	//Previous char offset

	for (uint curchar = 0; curchar < Path::MaxLength; curchar++)
	{
		if (m_path[curchar] == '/' || m_path[curchar] == 0)
		{
			if (idxcounter == idx)
			{
				return Path(string(m_path.get() + prevchar, curchar));
			}

			prevchar = curchar;

			idxcounter++;
		}
	}

	return Path();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Path::addDirectories(const char* dirstr)
{
	Path dir(dirstr);
	uint16 count = dir.getDirectoryCount();

	auto len = (uint16)strlen(m_path.get());

	for (uint16 i = 0; i < count; i++)
	{
		Path subdir(dir.getDirectory(i));

		m_path.at(len) = '/';

		m_path.set(subdir.str(), len + 1);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

Path Path::getRelativePath(const Path& subpath) const
{
	Path destpath(*this);
	
	//iterate over each directory in subpath
	uint16 numdirs = subpath.getDirectoryCount();
	for (uint16 i = 0; i < numdirs; i++)
	{
		string dirstr(subpath.getDirectory(i).str());
		
		//If the directory is ".." then remove the topmost directory from the path
		if (dirstr == "..")
		{
			destpath = destpath.getParent();
		}
		else
		{
			//Append a single directory
			destpath.addDirectories(dirstr);
		}
	}

	
	return destpath;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

Path Path::getParent() const
{
	StaticString<Path::MaxLength> parentbuf(m_path);

	size_t len = parentbuf.length();

	//Iterate over path backwards while erasing each character until the last path divider has been reached
	for (size_t i = len; i >= 0; i--)
	{
		if (parentbuf[i] == '/')
		{
			break;
		}

		if (i == 0)
		{
			return Path();
		}

		parentbuf[i] = 0;
	}

	return Path(parentbuf.get());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////