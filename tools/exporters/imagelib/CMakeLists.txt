#
#   Model Lib
#

set(imagelib_src
    binding.cpp
)

add_pybind_module(imagelib ${imagelib_src})

target_link_libraries(imagelib PRIVATE tsgraphics tscore ${RCS_GEN_LIBRARIES})
