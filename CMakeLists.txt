################################################################################################
#	TS-Engine
################################################################################################

CMAKE_MINIMUM_REQUIRED (VERSION 3.8)

PROJECT (tse)

if("${PROJECT_BINARY_DIR}" STREQUAL "${PROJECT_SOURCE_DIR}")
	message(FATAL_ERROR "In-source builds are not permitted")
endif()

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    #set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "default install path" FORCE )
endif()

#set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "default install path" FORCE )

################################################################################################

# Cache options
option(TS_BUILD_TESTS "build tests" ON)
option(TS_BUILD_SAMPLES "build sample applications" ON)

# Display IDE folders
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

# CMake module locations
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Override default project outputs
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/")
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/")
SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib/")

set(TS_BINARY_INSTALL "bin")
set(TS_LIBRARY_INSTALL "lib")
set(TS_HEADER_INSTALL "include")

# System Libs
set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS 1)
set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
set(CMAKE_INSTALL_DEBUG_LIBRARIES 0)
include(InstallRequiredSystemLibraries)

################################################################################################
#	configuration file
################################################################################################

# git information
include(GetGitRevisionDescription)
include(GetGitCommitDescription)

get_git_head_revision(GIT_REFSPEC GIT_SHA1)
get_git_commit_desc(GIT_DATE GIT_COMMIT_SUBJECT)

#create the configuration file
CONFIGURE_FILE (
	"${PROJECT_SOURCE_DIR}/tsconfig.h.in"
	"${PROJECT_BINARY_DIR}/config/tsconfig.h"
)

#create version info file
CONFIGURE_FILE(
	"${PROJECT_SOURCE_DIR}/tsversion.h.in"
	"${PROJECT_BINARY_DIR}/config/tsversion.h"
)

INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}/config")

################################################################################################
# External dependencies
################################################################################################

# Include external dependencies
include(${PROJECT_SOURCE_DIR}/external/External.cmake)

# Setup DataBuild tool
include(DataBuildTool)

if(TS_BUILD_TESTS)
	enable_testing()
endif()

# Engine modules
ADD_SUBDIRECTORY(modules)

# Engine tools
ADD_SUBDIRECTORY(tools)

# Engine samples
if (TS_BUILD_SAMPLES)
    ADD_SUBDIRECTORY(samples)
endif()

################################################################################################
