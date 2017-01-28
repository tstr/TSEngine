/*
	Shader Info Parser:

	parses a shader manifest file describing how shaders are to be compiled

	.shm
*/

#pragma once

#include "../common.h"

#include <tscore/filesystem/path.h>
#include <tscore/system/memory.h>

/////////////////////////////////////////////////////////////////////////////////////////////////

class CShaderInfoParser
{
private:

	struct Impl;
	Impl* pImpl = nullptr;

public:

	CShaderInfoParser() : pImpl(nullptr) {}
	CShaderInfoParser(const std::string& data);
	CShaderInfoParser(const char* data, std::size_t datalen);
	CShaderInfoParser(const ts::Path& path);
	
	CShaderInfoParser(const CShaderInfoParser&) = delete;
	operator bool() const { return (pImpl != nullptr); }
	
	CShaderInfoParser(CShaderInfoParser&& imp)
	{
		Impl* i = this->pImpl;
		this->pImpl = imp.pImpl;
		imp.pImpl = i;
	}

	~CShaderInfoParser();
	
	bool loadData(const std::string& data);
	bool loadData(const char* data, std::size_t datalen);
	bool load(const ts::Path& path);

	ts::uint32 getShaderCount() const;
	void getShaderInfo(ts::uint32 idx, std::string& shaderName, SShaderInfo& info) const;
	void findShaderInfo(const std::string& shaderName, SShaderInfo& info) const;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
