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
ADD_SUBDIRECTORY(CLIutil)
ADD_SUBDIRECTORY(rcschema)
ADD_SUBDIRECTORY(modelbuild)
ADD_SUBDIRECTORY(shaderc)

# resource connectors
add_subdirectory(connectors)

SET_TARGET_PROPERTIES(
	rcschema
	CLIutil
	modelbuild
	shaderc
	PROPERTIES FOLDER tools
)

################################################################################################
