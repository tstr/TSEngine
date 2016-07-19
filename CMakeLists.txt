
CMAKE_MINIMUM_REQUIRED (VERSION 3.0)

PROJECT (CTEngine)

################################################################################################

#configuration file
CONFIGURE_FILE (
	"${PROJECT_SOURCE_DIR}/config.h.src"
	"${PROJECT_BINARY_DIR}/config.h"
)

INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}/")

################################################################################################


INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)

SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/)
SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/)
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/)

# SET (CMAKE_LIBRARY_PATH
	# ${CMAKE_LIBRARY_PATH}
	# ${PROJECT_SOURCE_DIR}/dependencies
	# ${PROJECT_BINARY_DIR}/lib
# )

LINK_DIRECTORIES(
	${PROJECT_SOURCE_DIR}/dependencies
	${PROJECT_BINARY_DIR}/lib
)

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

#engine components
ADD_SUBDIRECTORY(source/ctapp)
ADD_SUBDIRECTORY(source/ctcore)
ADD_SUBDIRECTORY(source/ctgraphics)
ADD_SUBDIRECTORY(source/ctgraphicsD3D11)

#engine tools
ADD_SUBDIRECTORY(tools/modelbuilder)

################################################################################################