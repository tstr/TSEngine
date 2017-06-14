################################################################################################
#	TS-Engine
################################################################################################

CMAKE_MINIMUM_REQUIRED (VERSION 3.0)

PROJECT (tse)

################################################################################################

OPTION(TS_TEST "enable tests" OFF)
#OPTION(TS_BUILD_TOOLS "build tools" ON)

IF(TS_TEST)
	
	enable_testing()
	
ENDIF()

if("${PROJECT_BINARY_DIR}" STREQUAL "${PROJECT_SOURCE_DIR}")
	message(FATAL_ERROR "In-source builds are not permitted")
endif()

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/")
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/")
SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib/")

set (TS_BINARY_INSTALL "${CMAKE_INSTALL_PREFIX}/bin")
set (TS_LIBRARY_INSTALL "${CMAKE_INSTALL_PREFIX}/lib")
set (TS_HEADER_INSTALL "${CMAKE_INSTALL_PREFIX}/inc")

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
#	Automatic shader generator
################################################################################################

# In order to get around issues with variable scope - use global properties instead of regular variables
SET_PROPERTY(GLOBAL PROPERTY SHADER_GENERATOR_MANIFEST_LIST "")
SET_PROPERTY(GLOBAL PROPERTY SHADER_GENERATOR_SOURCE_LIST "")

# Name for target
set (SHADER_GENERATOR_TARGET PrecompileShaders)

# Directories
set (SHADER_GENERATOR_LOG_FILE "${PROJECT_BINARY_DIR}/shadergen/out.log")
set (SHADER_GENERATOR_OUTPUT_DIR "${PROJECT_SOURCE_DIR}/assets/shaderbin/")
set (SHADER_GENERATOR_STAMP_DIR "${PROJECT_BINARY_DIR}/shadergen/stamp/")

# append a shader manifest and associated shader files to the generator list
function(WrapShaders)
	
	set(options)
	set(args MANIFEST)
	set(argsMulti SOURCES)
	cmake_parse_arguments(WrapShaders "${options}" "${args}" "${argsMulti}" ${ARGN} )
	
	GET_PROPERTY(manifest_list GLOBAL PROPERTY SHADER_GENERATOR_MANIFEST_LIST)
	GET_PROPERTY(source_list GLOBAL PROPERTY SHADER_GENERATOR_SOURCE_LIST)
	
	# cache absolute path of manifest
	get_filename_component(manifest_abs "${WrapShaders_MANIFEST}" ABSOLUTE)
	list(APPEND manifest_list ${manifest_abs})
	#set(manifest_list ${manifest_list} ${manifest_abs})
	
	foreach (src ${WrapShaders_SOURCES})
		
		# cache each absolute source file path
		get_filename_component(src_abs "${src}" ABSOLUTE)
		list(APPEND source_list ${src_abs})
		#set(source_list ${source_list} ${src_abs})

	endforeach()
	
	SET_PROPERTY(GLOBAL PROPERTY SHADER_GENERATOR_MANIFEST_LIST ${manifest_list})
	SET_PROPERTY(GLOBAL PROPERTY SHADER_GENERATOR_SOURCE_LIST ${source_list})
	
endfunction(WrapShaders)

# set up target for running the compiler - called at end of list file
function(InitShaderGenerator)
	
	set(manifest_stamp_list "")
	
	GET_PROPERTY(manifest_list GLOBAL PROPERTY SHADER_GENERATOR_MANIFEST_LIST)
	GET_PROPERTY(source_list GLOBAL PROPERTY SHADER_GENERATOR_SOURCE_LIST)
	
	list(REMOVE_DUPLICATES manifest_list)
	list(REMOVE_DUPLICATES source_list)

	foreach(m ${manifest_list})# ${shadercomp_sources})
		
		get_filename_component(manifest "${m}" NAME)
		list(APPEND manifest_stamp_list "${SHADER_GENERATOR_STAMP_DIR}/${manifest}.stamp")
		
	endforeach()
	
	add_custom_target(
		${SHADER_GENERATOR_TARGET} ALL
		#SOURCES ${source_list} ${manifest_list}
		DEPENDS ${manifest_stamp_list}
	)

	add_dependencies(${SHADER_GENERATOR_TARGET} shaderc)
	
	set_target_properties(
		${SHADER_GENERATOR_TARGET}
		PROPERTIES FOLDER ${SAMPLES_FOLDER}
	)
	
	#Foreach manifest
	foreach(m ${manifest_list})
		
		get_filename_component(manifest "${m}" NAME)
		get_filename_component(manifestdir "${m}" DIRECTORY)
		set(manifest_stamp "${SHADER_GENERATOR_STAMP_DIR}/${manifest}.stamp")
		
		add_custom_command(
			OUTPUT ${manifest_stamp}					 # stamp file
			COMMAND cmake -E touch "${manifest_stamp}"	 # update stamp file
			COMMAND $<TARGET_FILE:shaderc> -t "${m}" -s "${manifestdir}" -o "${SHADER_GENERATOR_OUTPUT_DIR}" >> "${SHADER_GENERATOR_LOG_FILE}"
			DEPENDS ${m} ${source_list} # when either the manifest or sources are modified run command
			VERBATIM
		)
		
	endforeach()
		
endfunction(InitShaderGenerator)

################################################################################################
#	functions
################################################################################################

include(GenerateExportHeader)

SET(TS_BUILD_SHARED_LIBRARIES CACHE BOOL OFF)

#Add a component library
function(add_engine_library)
	
	set(options)
	set(args NAME HEADER_DIR)
	set(argsMulti SOURCES HEADERS)
	cmake_parse_arguments(add_engine_library "${options}" "${args}" "${argsMulti}" ${ARGN})
	
	set(libname ${add_engine_library_NAME})
	set(libsrcs ${add_engine_library_SOURCES})
	set(libhead ${add_engine_library_HEADERS})
	
	set(libtype STATIC)
	
	if (TS_BUILD_SHARED_LIBRARIES)
		set (libtype SHARED)
	endif()
	
	add_library(
		${libname}
		${libtype}
		${libsrcs} ${libhead}
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
	
	if (NOT add_engine_library_HEADER_DIR STREQUAL "")
	
		INSTALL(
			DIRECTORY "${add_engine_library_HEADER_DIR}/"
			DESTINATION ${TS_HEADER_INSTALL}
		)
		
	endif()
	
endfunction()

#Add a component executable
function(add_sample)
	
	set(options)
	set(args NAME SHADER_MANIFEST)
	set(argsMulti SOURCES SHADERS)
	cmake_parse_arguments(add_sample "${options}" "${args}" "${argsMulti}" ${ARGN})
	
	ADD_EXECUTABLE(
		${add_sample_NAME}
		${add_sample_SOURCES} ${add_sample_SHADERS} ${add_sample_SHADER_MANIFEST}
	)
		
	if (NOT ${add_sample_SHADER_MANIFEST} STREQUAL "")
		
		WrapShaders(MANIFEST ${add_sample_SHADER_MANIFEST} SOURCES ${add_sample_SHADERS})
		
	endif()
	
	INSTALL(
		TARGETS ${add_sample_NAME}
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

set(SAMPLES_FOLDER samples)
set(ENGINE_FOLDER engine)
set(TOOL_FOLDER tools)
set(TEST_FOLDER tests)

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

#Sample applications
ADD_SUBDIRECTORY(samples/Sandbox)
ADD_SUBDIRECTORY(samples/SampleTess)

SET_TARGET_PROPERTIES(
	Sandbox
	SampleTess
	PROPERTIES FOLDER ${SAMPLES_FOLDER}
)

#engine components
ADD_SUBDIRECTORY(engine/tscore)
ADD_SUBDIRECTORY(engine/tsengine)
ADD_SUBDIRECTORY(engine/tsgraphics)

SET_TARGET_PROPERTIES(
	tscore
	tsengine
	tsgraphics
	PROPERTIES FOLDER ${ENGINE_FOLDER}
)

################################################################################################

#engine tools
ADD_SUBDIRECTORY(tools/modelbuild)
ADD_SUBDIRECTORY(tools/shaderc)
ADD_SUBDIRECTORY(tools/texbuild)

SET_TARGET_PROPERTIES(
	modelbuild
	shaderc
	texbuild
	PROPERTIES FOLDER ${TOOL_FOLDER}
)

SET_TARGET_PROPERTIES(
	TestShaderc
	PROPERTIES FOLDER tests
)

################################################################################################
#	Automatic shader compilation

ADD_DEPENDENCIES(Sandbox ${SHADER_GENERATOR_TARGET})
ADD_DEPENDENCIES(SampleTess ${SHADER_GENERATOR_TARGET})

InitShaderGenerator()

################################################################################################
