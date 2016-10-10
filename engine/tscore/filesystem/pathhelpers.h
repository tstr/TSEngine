/*
	Path helper functions
*/

#pragma once

#include "path.h"

namespace ts
{
	bool isAbsolutePath(const Path& path); //Checks if the path is an absolute path

	bool isFile(const Path& path);		//Checks if the path is a file
	bool isDirectory(const Path& path); //Checks if the path is a folder (directory)

	//Searchs for a file in a set of directories
	bool resolveFile(const Path& filepath, Path& resultpath, const Path* searchPathArray, size_t searchPathArraySize);
	//Searchs for a file in a set of directories AND their subdirectories
	bool searchFile(const Path& filepath, Path& resultpath, const Path* searchPathArray, size_t searchPathArraySize);
}