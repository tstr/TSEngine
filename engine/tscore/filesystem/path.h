/*
	Path class header
	
	The path class is a string class that represents file system paths
*/

#pragma once

#include <tscore/strings.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	class TSCORE_API Path
	{
	public:
		
		enum
		{
			//The maximum allowed file path length
			MaxLength = 256
		};
		
		//constructors
		
		Path() {}
		~Path() {}
		
		Path(const Path& path)
		{
			m_path = path.m_path;
		}
		
		Path(const char* pathstr)
		{
			composePath(pathstr);
		}
		
		Path(std::string&& pathstr) :
			Path(pathstr.c_str())
		{}
		
		Path(const std::string& pathstr) :
			Path(pathstr.c_str())
		{}

		//operator overloads

		bool operator==(const Path& p) const
		{
			return compare_string_weak(p.m_path.str(), m_path.str());
		}

		bool operator!=(const Path& p) const
		{
			return !this->operator==(p);
		}
		
		//methods

		const char* str() const { return m_path.str(); } 
		void str(std::string& str) const { str = m_path.str(); }
		
		void composePath(const std::string& str) { composePath(str.c_str()); }
		void composePath(const char* str);

		//Get parent folder
		Path getParent() const;

		//Count number of subdirectories in this path
		uint16 getDirectoryCount() const;

		//Add a directory or group of directories to this path
		void addDirectories(const Path& dir);

		//Get the name of a directory at a particular index of the path
		Path getDirectory(uint16 idx) const;

		//Get the top-most directory in the path
		inline Path getDirectoryTop() const { return getDirectory(getDirectoryCount() - 1); }
		//Get the bottom-most directory in the path
		inline Path getDirectoryRoot() const { return getDirectory(0); }

	private:
		
		StaticString<Path::MaxLength> m_path;
	};


}

//Hash function
namespace std
{
	template<>
	struct hash<ts::Path>
	{
		size_t operator()(ts::Path path) const
		{
			hash<const char*>h;
			return h(path.str());
		}
	};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
