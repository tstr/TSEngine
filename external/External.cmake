#
#   External dependency management
#

# Include guard
if(DEFINED __EXTERNAL_INCLUDED)
    return()
endif()
set(__EXTERNAL_INCLUDED TRUE)

# Workaround for not being able to alias imported targets easily
macro(add_imported_alias _alias _target)
    add_library(${_alias} INTERFACE IMPORTED)
    foreach(_name INTERFACE_LINK_LIBRARIES INTERFACE_INCLUDE_DIRECTORIES INTERFACE_COMPILE_DEFINITIONS INTERFACE_COMPILE_OPTIONS)
        get_property(_value TARGET ${_target} PROPERTY ${_name} )
        set_property(TARGET ${_alias} PROPERTY ${_name} ${_value})
    endforeach()
endmacro()


find_package(PythonInterp REQUIRED)
#find_package(PythonLibs REQUIRED)
find_package(PythonLibsNew REQUIRED)

if (NOT PYTHONLIBS_FOUND)
	message(FATAL_ERROR "Unable to find python libs")
endif()

option (TS_USE_CONAN "Use Conan to fetch dependencies" TRUE)

if (TS_USE_CONAN)

    # Include conan helper script
    include("${CMAKE_CURRENT_LIST_DIR}/conan.cmake")

    # Find conan executable
    find_program(CONAN_COMMAND conan)

    if (NOT CONAN_COMMAND)
        message(FATAL_ERROR "Conan command not found.")
    endif()

    mark_as_advanced(CONAN_COMMAND)

    # Setup remotes
    execute_process(
        COMMAND ${CONAN_COMMAND} remote list
        OUTPUT_VARIABLE _conan_remotes
    )

    # Add required remotes
    if(NOT "${_conan_remotes}" MATCHES ".*bincrafters:.*")
        execute_process(
            COMMAND
            ${CONAN_COMMAND} remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
        )
    endif()

    # Run Conan
    conan_cmake_run(
        CONANFILE "conanfile.txt"
        BASIC_SETUP CMAKE_TARGETS NO_OUTPUT_DIRS
    )
	
    # Set aliases
    add_imported_alias(assimp CONAN_PKG::Assimp)
    add_imported_alias(pybind11 CONAN_PKG::pybind11)

else()

    find_package(pybind11 REQUIRED)
    find_package(assimp REQUIRED)

endif()
