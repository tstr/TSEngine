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
ADD_SUBDIRECTORY(texbuild)

SET_TARGET_PROPERTIES(
	rcschema
	CLIutil
	modelbuild
	shaderc
	texbuild
	PROPERTIES FOLDER tools
)

################################################################################################
