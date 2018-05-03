#
#   Model Lib
#

set(texlib_src
    binding.cpp
)

add_pybind_module(texturelib ${texlib_src})

target_link_libraries(texturelib PRIVATE tsgraphics tscore ${RCS_GEN_LIBRARIES})
