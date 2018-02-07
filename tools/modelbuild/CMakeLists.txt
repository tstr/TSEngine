#
#	ModelBuild:
#	Tool for converting model files
#

#####################################################################################################################
#	Model builder project
#####################################################################################################################

SET (modelbuild_src
	src/build.cpp
)

ADD_EXECUTABLE(
	modelbuild
	${modelbuild_src}
)

install_tools(modelbuild)

assign_source_groups(${modelbuild_src})

target_link_libraries(modelbuild
    PRIVATE
        tscore
        tsgraphics
        assimp
)

#####################################################################################################################

# ADD_CUSTOM_TARGET(modelbuild_importer ALL)

# ADD_CUSTOM_COMMAND(
	# TARGET modelbuild POST_BUILD
	# COMMAND "$<TARGET_FILE_DIR:modelbuild>/modelbuild.exe" -t = "${PROJECT_SOURCE_DIR}/cube.obj" -o = "${PROJECT_BINARY_DIR}"
# )

# SET_TARGET_PROPERTIES(modelbuild_importer PROPERTIES FOLDER tools)

#####################################################################################################################
