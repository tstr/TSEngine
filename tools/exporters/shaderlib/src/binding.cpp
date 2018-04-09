/*
    ShaderLib python bindings
*/

#include <pybind11/pybind11.h>

namespace py = pybind11;

int testFunc(int x, int y)
{
    return x + y;
}

PYBIND11_MODULE(shaderlib, m)
{
    m.def("test_func", &testFunc);
}
