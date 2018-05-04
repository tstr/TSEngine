/*
    ShaderLib python bindings
*/

#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>

#include "ShaderSource.h"

namespace py = pybind11;

using namespace std;
using namespace ts;

PYBIND11_MODULE(shaderlib, m)
{
	py::class_<ShaderSource> sourceClass(m, "Source");

	sourceClass
		.def(py::init<const std::string&>())
		.def("load", &ShaderSource::load)
		.def("compile", &ShaderSource::compile)
		.def("dependencies", &ShaderSource::dependencyNames)
		.def("source_name", &ShaderSource::sourceName)
		.def("source_text", &ShaderSource::sourceText)
		.def("error_string", &ShaderSource::errorString)
		.def("error_code", &ShaderSource::errorCode);

	py::enum_<ShaderSource::Error>(sourceClass, "Error")
		.value("OK", ShaderSource::OK)
		.value("PREPROCESSOR_ERROR", ShaderSource::PREPROCESSOR_ERROR)
		.value("FILE_NOT_FOUND_ERROR", ShaderSource::FILE_NOT_FOUND_ERROR)
		.value("COMPILER_ERROR", ShaderSource::COMPILER_ERROR)
		.value("METAPARSER_ERROR", ShaderSource::METAPARSER_ERROR)
		.export_values();
}
