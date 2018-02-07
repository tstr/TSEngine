/*
	Path class source

	todo:

	- filter special characters
	- iterators
*/

#include <tscore/path.h>

#define NOMINMAX
#include <Windows.h>

#include <algorithm>

using namespace ts;
using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Path::set(const Path& path)
{
	this->composePath(path.str());
}

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

	if (length > 0)
	{
		if (strbuf[length - 1] == '\\' || strbuf[length - 1] == '/')
		{
			strbuf[length - 1] = 0;
		}
	}


	m_path.set(strbuf.str() + offset);
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
	uint prevchar = 0;	//Previous char offset

	for (uint curchar = 0; curchar < Path::MaxLength; curchar++)
	{
		if (m_path[curchar] == '/' || m_path[curchar] == 0)
		{
			if (idxcounter == idx)
			{
				auto str = string(m_path.str() + prevchar, m_path.str() + curchar);
				return Path(move(str));
			}

			prevchar = curchar + 1;

			idxcounter++;
		}
	}

	return Path();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Path::addDirectories(const Path& subpath)
{
	//iterate over each directory in subpath
	uint16 numdirs = subpath.getDirectoryCount();

	for (uint16 i = 0; i < numdirs; i++)
	{
		string dirstr(trim(subpath.getDirectory(i).str()));
		
		//If the directory is ".." then remove the topmost directory from the path
		if (dirstr == "..")
		{
			this->composePath(this->getParent().str());
		}
		else
		{
			//Append a single directory
			size_t len = m_path.length();
			m_path.set("/", len);
			m_path.set(dirstr.c_str(), len + 1);
		}
	}	
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

	return Path(parentbuf.str());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////