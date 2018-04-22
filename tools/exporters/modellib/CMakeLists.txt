#
#   Model Lib
#

set(modellib_src
    src/Builder.h
    src/Builder.cpp
    src/binding.cpp
)

add_pybind_module(modellib ${modellib_src})

assign_source_groups(${modellib_src})

target_link_libraries(modellib PRIVATE assimp tsgraphics)

add_dependencies(DATA_BUILD modellib)

set_target_properties(modellib PROPERTIES FOLDER tools/extensions)
