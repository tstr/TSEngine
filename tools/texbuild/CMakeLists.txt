#
#	TexBuild:
#	Tool for converting images to textures
#

#####################################################################################################################
#	Texture Converter project
#####################################################################################################################

SET (texbuild_src
	src/main.cpp
)

ADD_EXECUTABLE(
	texbuild
	${texbuild_src}
)

INSTALL(
	TARGETS texbuild
	RUNTIME DESTINATION ${TS_BINARY_INSTALL}
	LIBRARY DESTINATION ${TS_BINARY_INSTALL}
	ARCHIVE DESTINATION ${TS_LIBRARY_INSTALL}
)

assign_source_groups(${texbuild_src})

target_link_libraries(
	texbuild PRIVATE
	tsgraphics
)

#####################################################################################################################
