#
#   Model Lib
#

set(modellib_src
    src/Builder.h
    src/Builder.cpp
    src/binding.cpp
)

add_pybind_module(modellib ${modellib_src})

target_link_libraries(modellib PRIVATE assimp tsgraphics)
