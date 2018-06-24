#
#   Resource connectors
#

include(PythonUtils)

# Hook function to setup installation rules for python extensions
function(add_pybind_module _target)

    _add_pybind_module(${_target} ${ARGN})
    
    assign_source_groups(${ARGN})
    
    add_dependencies(DATA_BUILD ${_target})

    set_target_properties(${_target} PROPERTIES FOLDER "tools/extensions")
    
    install(TARGETS ${_target}
        RUNTIME DESTINATION "tools/exporters"
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
	)
    
    install_target_pdb(${_target})
endfunction()

add_subdirectory(shaderlib)
add_subdirectory(modellib)
add_subdirectory(imagelib)

FILE(GLOB _python_modules "${CMAKE_CURRENT_SOURCE_DIR}/*.py")

# Install python modules
install(
    FILES ${_python_modules}
    DESTINATION "tools/exporters"
)
