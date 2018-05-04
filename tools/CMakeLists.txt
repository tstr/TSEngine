################################################################################################
#
#	Tools
#
################################################################################################

include(CommonHelpers)

function(install_tools)
	INSTALL(
		TARGETS ${ARGV}
		RUNTIME DESTINATION "tools"
		LIBRARY DESTINATION "tools"
		ARCHIVE DESTINATION "tools"
	)
	
	foreach(_target ${ARGV})
		install_target_pdb(${_target})
	endforeach()
	
endfunction()

#engine tools
add_subdirectory(CLIutil)
add_subdirectory(rcschema)
add_subdirectory(exporters)

SET_TARGET_PROPERTIES(
	rcschema
	CLIutil
	PROPERTIES FOLDER tools
)

################################################################################################

# Databuild tool install rules
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/dbuild.py" DESTINATION "tools")

file(GLOB _python_modules "${CMAKE_CURRENT_SOURCE_DIR}/databuild/*.py")

# Install python modules
install(
    FILES ${_python_modules}
    DESTINATION "tools/databuild/"
)


################################################################################################
