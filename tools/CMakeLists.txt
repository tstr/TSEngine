################################################################################################
#
#	Tools
#
################################################################################################

#engine tools
ADD_SUBDIRECTORY(modelbuild)
ADD_SUBDIRECTORY(shaderc)
ADD_SUBDIRECTORY(texbuild)

SET_TARGET_PROPERTIES(
	modelbuild
	shaderc
	texbuild
	PROPERTIES FOLDER tools
)

#tests
SET_TARGET_PROPERTIES(
	TestShaderc
	PROPERTIES FOLDER tests
)

################################################################################################
