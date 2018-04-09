#
#   Python utilities
#

if (NOT DEFINED PYTHON_MODULE_EXTENSION)
    set(PYTHON_MODULE_EXTENSION ".pyd")
endif()

if (NOT DEFINED PYTHON_MODULE_PREFIX)
    set(PYTHON_MODULE_PREFIX "")
endif()

#
#   Add pybind module
#
macro(add_pybind_module _target)
    add_library(${_target} SHARED ${ARGN})
    set_target_properties(${_target} PROPERTIES PREFIX "${PYTHON_MODULE_PREFIX}")
    set_target_properties(${_target} PROPERTIES SUFFIX "${PYTHON_MODULE_EXTENSION}")
    target_link_libraries(${_target} PRIVATE pybind11)
endmacro()


# Determine file separator
if("${CMAKE_HOST_SYSTEM}" MATCHES ".*Windows.*")
    set(_sep "\\;")
else()
    set(_sep ":")
endif()

# Environment wrapper, configures PYTHONPATH:
#   - adds runtime output dir
#   - adds project root
set(PYWRAPPER_COMMAND
    "${CMAKE_COMMAND}" -E env "PYTHONPATH=${PROJECT_BINARY_DIR}/bin/$<CONFIG>/${_sep}${PROJECT_SOURCE_DIR}${_sep}$ENV{PYTHONPATH}"
    "${PYTHON_EXECUTABLE}"
)
