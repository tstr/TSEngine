################################################################################################
#
#	Tools
#
################################################################################################

include(CommonHelpers)

function(install_tools)
	INSTALL(
		TARGETS ${ARGV}
		RUNTIME DESTINATION "${TS_BINARY_INSTALL}"
		LIBRARY DESTINATION "${TS_BINARY_INSTALL}"
		ARCHIVE DESTINATION "${TS_LIBRARY_INSTALL}"
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
