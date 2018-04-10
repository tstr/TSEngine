#
#   Model Lib
#

include(PythonUtils)

add_pybind_module(modellib src/binding.cpp)

target_link_libraries(modellib PRIVATE assimp)

add_dependencies(DATA_BUILD modellib)

set_target_properties(modellib PROPERTIES FOLDER tools/extensions)
