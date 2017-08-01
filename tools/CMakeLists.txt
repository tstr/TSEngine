################################################################################################
#
#	Tools
#
################################################################################################

function(install_tools)
	INSTALL(
		TARGETS ${ARGV}
		RUNTIME DESTINATION "${TS_BINARY_INSTALL}/tools"
		LIBRARY DESTINATION "${TS_BINARY_INSTALL}/tools"
		ARCHIVE DESTINATION "${TS_LIBRARY_INSTALL}/tools"
	)
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
