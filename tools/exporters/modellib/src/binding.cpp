/*
    ShaderLib python bindings
*/

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>

#include "Builder.h"

namespace py = pybind11;

PYBIND11_MODULE(modellib, m)
{
	using Redirect = py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>;

	py::class_<Model>(m, "ModelBuilder")
		.def(py::init<>(), Redirect())
		.def("imp", &Model::imp, Redirect())
		.def("exp", &Model::exp, Redirect())
		.def("supported_extensions", &Model::getExtensions)
		.def("error_string", &Model::getErrorString);
}
