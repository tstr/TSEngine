################################################################################################
#	TS-Engine
################################################################################################

CMAKE_MINIMUM_REQUIRED (VERSION 3.0)

PROJECT (tse)

################################################################################################

if("${PROJECT_BINARY_DIR}" STREQUAL "${PROJECT_SOURCE_DIR}")
	message(FATAL_ERROR "In-source builds are not permitted")
endif()

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/")
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/")
SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib/")

set (TS_BINARY_INSTALL "${CMAKE_INSTALL_PREFIX}/bin")
set (TS_LIBRARY_INSTALL "${CMAKE_INSTALL_PREFIX}/lib")
set (TS_INCLUDE_INSTALL "${CMAKE_INSTALL_PREFIX}/include")

# Project include directory
INCLUDE_DIRECTORIES("${PROJECT_BINARY_DIR}/inc")

# System Libs
set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS 1)
set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "${BIN_INSTALL}")
set(CMAKE_INSTALL_DEBUG_LIBRARIES 0)
include(InstallRequiredSystemLibraries)

# Module path
SET (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmakemodules")

################################################################################################
#	functions
################################################################################################

include(GenerateExportHeader)

SET(TS_BUILD_SHARED_LIBRARIES CACHE BOOL OFF)

#Add a component library
function(add_library_tse libname libsrcs)
	
	set(libtype STATIC)
	
	if (TS_BUILD_SHARED_LIBRARIES)
		set (libtype SHARED)
	endif()
	
	add_library(
		${libname}
		${libtype}
		${libsrcs}
	)
	
	set(macro_libname ${libname})
	string(TOUPPER ${macro_libname} macro_libname)
	
	GENERATE_EXPORT_HEADER(
		${libname}
		STATIC_DEFINE TS_LIBRARY_STATIC
		EXPORT_MACRO_NAME "${macro_libname}_API"
		NO_EXPORT_MACRO_NAME "${macro_libname}_API_NO"
	)
	
	#Add location of libname_export.h file to public interface
	TARGET_INCLUDE_DIRECTORIES(
		${libname}
		PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
	)
	
	SET_TARGET_PROPERTIES(${libname} PROPERTIES LINKER_LANGUAGE CXX)
	
	INSTALL(
		TARGETS ${libname}
		RUNTIME DESTINATION ${TS_BINARY_INSTALL}
		LIBRARY DESTINATION ${TS_BINARY_INSTALL}
		ARCHIVE DESTINATION ${TS_LIBRARY_INSTALL}
	)
	
endfunction()

#Add a component executable
function(add_executable_tse libname libsrcs)
	
	ADD_EXECUTABLE(
		${libname}
		${libsrcs}
	)
	
	INSTALL(
		TARGETS ${libname}
		RUNTIME DESTINATION ${TS_BINARY_INSTALL}
		LIBRARY DESTINATION ${TS_BINARY_INSTALL}
		ARCHIVE DESTINATION ${TS_LIBRARY_INSTALL}
	)
	
endfunction()

#Create source groups based on the directory structure
function(assign_source_groups)
	foreach(file ${ARGN})
		
		#Get parent directory
		get_filename_component(source_dir "${file}" DIRECTORY)
		
		#Make sure we are using windows slashes
		string(REPLACE "/" "\\" source_dir "${source_dir}")
		
		source_group("${source_dir}" FILES "${file}")
		
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
#	Projects
################################################################################################

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

#applications
ADD_SUBDIRECTORY(app/sceneviewer)
ADD_SUBDIRECTORY(app/rendertest)

SET_TARGET_PROPERTIES(
	sceneviewer
	rendertest
	PROPERTIES FOLDER applications
)

#engine components
ADD_SUBDIRECTORY(engine/tscore)
ADD_SUBDIRECTORY(engine/tsengine)
ADD_SUBDIRECTORY(engine/tsgraphics)

SET_TARGET_PROPERTIES(
	tscore
	tsengine
	tsgraphics
	PROPERTIES FOLDER engine
)

################################################################################################

OPTION(TS_TEST "enable tests" OFF)
OPTION(TS_BUILD_TOOLS "build tools" ON)

IF(TS_TEST)
	
	enable_testing()
	
ENDIF()

IF(TS_BUILD_TOOLS)

	#engine tools
	ADD_SUBDIRECTORY(tools/modelbuild)
	ADD_SUBDIRECTORY(tools/shaderc)
	ADD_SUBDIRECTORY(tools/texbuild)
	
	SET_TARGET_PROPERTIES(
		modelbuild
		shaderc
		texbuild
		PROPERTIES FOLDER tools
	)
	
ENDIF()

################################################################################################
