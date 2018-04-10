/*
    ShaderLib python bindings
*/

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <assimp/Importer.hpp>

namespace py = pybind11;

class ModelBuilder
{
private:

	//Assimp importer
    Assimp::Importer m_aiImporter;

	//Supported file extensions
	std::unordered_set<std::string> m_extensions;

public:

    ModelBuilder()
	{
		std::string ext;
		std::string extList;
		m_aiImporter.GetExtensionList(extList);

		auto it = extList.begin();

		//Parse ; separated list
		while (it != extList.end())
		{
			ext.clear();

			assert(*it == '*'); it++;
			assert(*it == '.'); it++;

			while (it != extList.end() && *it != ';')
			{
				ext += *it;
				it++;
			}

			if (it != extList.end() && *it == ';')
				it++;

			m_extensions.insert(ext);
		}
	}

    const std::unordered_set<std::string>& getExtensions()
    {
		return m_extensions;
    }
};

PYBIND11_MODULE(modellib, m)
{
	py::class_<ModelBuilder>(m, "ModelBuilder")
		.def(py::init<>())
		.def("supported_extensions", &ModelBuilder::getExtensions);
}
