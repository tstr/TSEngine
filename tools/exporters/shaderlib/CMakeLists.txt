#
#   Shader Lib
#

include(PythonUtils)

add_pybind_module(shaderlib src/binding.cpp)

add_dependencies(DATA_BUILD shaderlib)

set_target_properties(shaderlib PROPERTIES FOLDER tools/extensions)
