################################################################################################
#	TS-Engine
################################################################################################

CMAKE_MINIMUM_REQUIRED (VERSION 3.0)

PROJECT (tse)

################################################################################################

OPTION(TS_BUILD_TESTS "enable tests" OFF)
#OPTION(TS_BUILD_TOOLS "build tools" ON)

if(TS_BUILD_TESTS)
	enable_testing()
endif()

if("${PROJECT_BINARY_DIR}" STREQUAL "${PROJECT_SOURCE_DIR}")
	message(FATAL_ERROR "In-source builds are not permitted")
endif()

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/")
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/")
SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib/")

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(CMAKE_USE_RELATIVE_PATHS TRUE)

set(TS_BINARY_INSTALL "${CMAKE_INSTALL_PREFIX}/bin")
set(TS_LIBRARY_INSTALL "${CMAKE_INSTALL_PREFIX}/lib")
set(TS_HEADER_INSTALL "${CMAKE_INSTALL_PREFIX}/inc")

# System Libs
set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS 1)
set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "${BIN_INSTALL}")
set(CMAKE_INSTALL_DEBUG_LIBRARIES 0)
include(InstallRequiredSystemLibraries)

# Module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmakemodules")

################################################################################################
#	functions
################################################################################################

#Create source groups based on the directory structure
function(assign_source_groups)
	foreach(_file ${ARGN})
		
		#Resolve absolute path
		get_filename_component(source_file "${_file}" ABSOLUTE)
		
		#Attempt to determine if the file is in the source or build tree
		string(FIND "${source_file}" "${CMAKE_CURRENT_SOURCE_DIR}" is_in_src)
		string(FIND "${source_file}" "${CMAKE_CURRENT_BINARY_DIR}" is_in_build)
		
		#If this file is in the build tree
		if(is_in_build EQUAL 0)
			file(RELATIVE_PATH source_file ${CMAKE_CURRENT_BINARY_DIR} ${source_file})
		#Otherwise if this file is in the source tree
		elseif(is_in_src EQUAL 0)
			file(RELATIVE_PATH source_file ${CMAKE_CURRENT_SOURCE_DIR} ${source_file})
		endif()
		
		#Get parent directory
		get_filename_component(source_dir "${source_file}" DIRECTORY)
		
		#Make sure we are using windows slashes
		#string(REPLACE "/" "\\" source_dir "${source_dir}")
		file(TO_NATIVE_PATH "${source_dir}" source_dir)
		
		#Debug print
		#message("[${is_in_src}||${is_in_build}]${source_file}")
		
		source_group("${source_dir}" FILES "${_file}")
		
	endforeach()
endfunction()

################################################################################################
#	configuration file
################################################################################################

#git
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

#git executable path
set (GIT_EXECUTABLE git)

#the date of the commit
execute_process(COMMAND
  "${GIT_EXECUTABLE}" log -1 --format=%ad --date=local
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_DATE
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

#the subject of the commit
execute_process(COMMAND
  "${GIT_EXECUTABLE}" log -1 --format=%s
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_COMMIT_SUBJECT
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

#platform macros
if (WIN32)
	SET(TS_PLATFORM_WIN32 1)
endif()

if (UNIX)
	SET(TS_PLATFORM_UNIX 1)
endif()

if (APPLE)
	SET(TS_PLATFORM_APPLE 1)
endif()

#create the configuration file
CONFIGURE_FILE (
	"${PROJECT_SOURCE_DIR}/tsconfig.h.in"
	"${PROJECT_BINARY_DIR}/cfg/tsconfig.h"
)

#create version info file
CONFIGURE_FILE(
	"${PROJECT_SOURCE_DIR}/tsversion.h.in"
	"${PROJECT_BINARY_DIR}/cfg/tsversion.h"
)

INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}/cfg")

################################################################################################
#	projects
################################################################################################

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

# Engine libraries
ADD_SUBDIRECTORY(engine)

# Engine tools
ADD_SUBDIRECTORY(tools)

# Engine samples
ADD_SUBDIRECTORY(samples)

################################################################################################
