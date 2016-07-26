#
#	Tool for converting model files
#

SET (ModelBuilder_list
	build.cpp
)

ADD_EXECUTABLE(modelbuilder ${ModelBuilder_list})

SET_TARGET_PROPERTIES(modelbuilder PROPERTIES FOLDER tools)

FIND_LIBRARY(ASSIMP NAMES "assimp" PATHS ${PROJECT_SOURCE_DIR}/dependencies/)

TARGET_LINK_LIBRARIES(modelbuilder ASSIMP)