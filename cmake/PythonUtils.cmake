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
	target_link_libraries(${_target} PRIVATE ${PYTHON_LIBRARIES})
	target_include_directories(${_target} PRIVATE "${PYTHON_INCLUDE_DIR}")
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
function(format_pypath_cmd _output_var)
    set(_module_paths)
    foreach(_path ${ARGN})
        set(_module_paths "${_module_paths}${_sep}${_path}")
    endforeach()
    
    set(${_output_var} "${CMAKE_COMMAND}" -E env "PYTHONPATH=${_module_paths}$ENV{PYTHONPATH}" "${PYTHON_EXECUTABLE}" PARENT_SCOPE)
endfunction()

# Configure python path for tool scripts
format_pypath_cmd(
    PYWRAPPER_COMMAND
    "${PROJECT_BINARY_DIR}/bin/$<CONFIG>/"
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/tools/"
)
