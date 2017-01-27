/*
	Shader Definition file importer

	.tshc
*/

#pragma once

#include "../common.h"

#include <tscore/filesystem/path.h>
#include <tscore/system/memory.h>

/////////////////////////////////////////////////////////////////////////////////////////////////

class CShaderDefImporter
{
private:

	struct Impl;
	Impl* pImpl = nullptr;

public:

	CShaderDefImporter() : pImpl(nullptr) {}
	CShaderDefImporter(const std::string& data);
	CShaderDefImporter(const char* data, std::size_t datalen);
	CShaderDefImporter(const ts::Path& path);
	
	CShaderDefImporter(const CShaderDefImporter&) = delete;
	
	CShaderDefImporter(CShaderDefImporter&& imp)
	{
		Impl* i = this->pImpl;
		this->pImpl = imp.pImpl;
		imp.pImpl = i;
	}

	~CShaderDefImporter();
	
	bool loadData(const std::string& data);
	bool loadData(const char* data, std::size_t datalen);
	bool load(const ts::Path& path);

	ts::uint32 getShaderCount() const;
	void getShaderInfo(ts::uint32 idx, SShaderInfo& info) const;
	void findShaderInfo(const char* shaderName, SShaderInfo& info) const;

};

/////////////////////////////////////////////////////////////////////////////////////////////////
