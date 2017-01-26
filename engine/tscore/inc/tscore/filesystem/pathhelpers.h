/*
	Path helper functions
*/

#pragma once

#include "path.h"
#include <fstream>

namespace ts
{
	bool TSCORE_API isAbsolutePath(const Path& path); //Checks if the path is an absolute path

	bool TSCORE_API isFile(const Path& path);		//Checks if the path is a file
	bool TSCORE_API isDirectory(const Path& path); //Checks if the path is a folder (directory)

	//Searchs for a file in a set of directories
	bool TSCORE_API resolveFile(const Path& filepath, Path& resultpath, const Path* searchPathArray, size_t searchPathArraySize);
	//Searchs for a file in a set of directories AND their subdirectories
	bool TSCORE_API searchFile(const Path& filepath, Path& resultpath, const Path* searchPathArray, size_t searchPathArraySize);
	
	//Creates a filestream object and/or directory tree
	std::fstream TSCORE_API createFile(const Path& path, int flags);
	inline std::fstream createFile(const std::string& path, int flags) { return createFile(Path(path.c_str()), flags); }
}