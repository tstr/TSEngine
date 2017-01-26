/*
	Shader Definition file importer
	
	source
*/

#include "importer.h"

#include <tscore/filesystem/pathhelpers.h>

#include <iostream>
#include <sstream>
#include <fstream>

using namespace ts;
using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Internal implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct CShaderDefImporter::Impl
{


	Impl(istream& data)
	{

	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Ctor/dtor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CShaderDefImporter::CShaderDefImporter(const char* data, size_t datalen)
{
	loadData(data, datalen);
}

CShaderDefImporter::CShaderDefImporter(const std::string& data)
{
	loadData(data);
}

CShaderDefImporter::CShaderDefImporter(const ts::Path& path)
{
	load(path);
}

CShaderDefImporter::~CShaderDefImporter()
{
	if (pImpl)
	{
		delete pImpl;
		pImpl = nullptr;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CShaderDefImporter::loadData(const char* data, size_t datalen)
{
	if (pImpl)
	{
		delete pImpl;
	}

	stringstream stream;
	stream.write(data, datalen);

	pImpl = new Impl(stream);

	return true;
}

bool CShaderDefImporter::loadData(const string& data)
{
	return this->loadData(data.c_str(), data.size());
}

bool CShaderDefImporter::load(const Path& path)
{
	if (pImpl)
	{
		delete pImpl;
	}

	ifstream file(path.str());
	pImpl = new Impl(file);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32 CShaderDefImporter::getShaderCount() const
{
	return 0;
}

void CShaderDefImporter::getShaderInfo(uint32 idx, SShaderInfo& info) const
{

}

void CShaderDefImporter::findShaderInfo(const char* shaderName) const
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
