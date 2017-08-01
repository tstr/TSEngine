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

install_tools(texbuild)

assign_source_groups(${texbuild_src})

target_link_libraries(
	texbuild PRIVATE
	tsgraphics
)

#####################################################################################################################
