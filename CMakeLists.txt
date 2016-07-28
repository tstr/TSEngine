
CMAKE_MINIMUM_REQUIRED (VERSION 3.0)

PROJECT (tse)

################################################################################################

#configuration file
CONFIGURE_FILE (
	"${PROJECT_SOURCE_DIR}/config.h.in"
	"${PROJECT_BINARY_DIR}/config.h"
)

INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}/")

################################################################################################

#sets project filters to match directory structure automatically
function(copy_source_groups)
	foreach(FILE ${ARGV}) 
		# Get the directory of the source file
		get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)

		# Remove common directory prefix to make the group
		string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")

		# Make sure we are using windows slashes
		string(REPLACE "/" "\\" GROUP "${GROUP}")

		set(GROUP "src\\${GROUP}")
		
		source_group("${GROUP}" FILES "${FILE}")
	endforeach()
endfunction()

################################################################################################


INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/engine)

SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/)
SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/)
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/)

SET (CMAKE_LIBRARY_PATH
	${CMAKE_LIBRARY_PATH}
	${PROJECT_SOURCE_DIR}/dependencies
	${PROJECT_BINARY_DIR}/lib
)

# LINK_DIRECTORIES(
	# ${PROJECT_SOURCE_DIR}/dependencies
	# ${PROJECT_BINARY_DIR}/lib
# )

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

#application
ADD_SUBDIRECTORY(app)

#engine components
ADD_SUBDIRECTORY(engine)

#engine tools
ADD_SUBDIRECTORY(tools/modelbuilder)

################################################################################################