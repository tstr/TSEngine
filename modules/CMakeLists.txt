################################################################################################
#
#	Engine Libraries
#
################################################################################################

include(CommonHelpers)
include(GenerateExportHeader)

SET(TS_BUILD_SHARED_LIBRARIES CACHE BOOL OFF)

set(MODULE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(MODULE_FOLDER modules)
set(MODULE_EXPORT_GROUP tsengine-modules)
set(MODULE_LIST)

#
#	Add an engine module library:
#
#	NAME		- name of target
#	SOURCES		- list of private sources
#	HEADERS		- list of public headers
#	PUBLIC_DIR	- public header directory
#	PRIVATE_DIR - private header directory
#
function(add_engine_module)
	
	set(options)
	set(args NAME PUBLIC_DIR PRIVATE_DIR)
	set(argsMulti SOURCES HEADERS)
	cmake_parse_arguments(param "${options}" "${args}" "${argsMulti}" ${ARGN})
	
	set(libname ${param_NAME})
	set(libsrcs ${param_SOURCES})
	set(libhdrs ${param_HEADERS})
	
	set(libtype STATIC)
	
	if (TS_BUILD_SHARED_LIBRARIES)
		set (libtype SHARED)
	endif()
	
	add_library(
		${libname}
		${libtype}
		${libsrcs} ${libhdrs}
	)
	
	list(APPEND MODULE_LIST ${libname})
	
	assign_source_groups(${libsrcs} ${libhdrs})
	
	string(TOUPPER ${libname} macro_libname)
	string(TOLOWER ${libname} export_filename)

	GENERATE_EXPORT_HEADER(
		${libname}
		STATIC_DEFINE TS_LIBRARY_STATIC
		EXPORT_MACRO_NAME "${macro_libname}_API"
		NO_EXPORT_MACRO_NAME "${macro_libname}_API_NO"
		EXPORT_FILE_NAME "${param_PUBLIC_DIR}/${export_filename}_export.h"
	)
	
	SET_TARGET_PROPERTIES(${libname} PROPERTIES LINKER_LANGUAGE CXX)
	
	INSTALL(
		TARGETS ${libname}
		EXPORT ${MODULE_EXPORT_GROUP}
		RUNTIME DESTINATION ${TS_BINARY_INSTALL}
		LIBRARY DESTINATION ${TS_BINARY_INSTALL}
		ARCHIVE DESTINATION ${TS_LIBRARY_INSTALL}
	)
    
    # Install PDB if it exists
	install_target_pdb(${libname})
	
	# If private directory specified
	if (NOT param_PRIVATE_DIR STREQUAL "")
		
		TARGET_INCLUDE_DIRECTORIES(${libname} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/${param_PRIVATE_DIR}")
		TARGET_INCLUDE_DIRECTORIES(${libname} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/${param_PRIVATE_DIR}")
		
		TARGET_INCLUDE_DIRECTORIES(${libname}
			PRIVATE
				"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${param_PRIVATE_DIR}>"
				"$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/${param_PRIVATE_DIR}>"
		)
		
	endif()
	
	# If public directory specified
	if (NOT param_PUBLIC_DIR STREQUAL "")
		
		TARGET_INCLUDE_DIRECTORIES(${libname}
			PUBLIC
				"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${param_PUBLIC_DIR}>"
				"$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/${param_PUBLIC_DIR}>"
				"$<INSTALL_INTERFACE:${TS_HEADER_INSTALL}>"
		)
		
		INSTALL(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${param_PUBLIC_DIR}/" DESTINATION ${TS_HEADER_INSTALL})
		
	endif()
	
	SET_TARGET_PROPERTIES(
		${libname}
		PROPERTIES FOLDER ${MODULE_FOLDER}
	)
	
endfunction()

################################################################################################
#	Engine modules
################################################################################################

ADD_SUBDIRECTORY(tscore)
ADD_SUBDIRECTORY(tsengine)
ADD_SUBDIRECTORY(tsgraphics)

################################################################################################

# Import file
# install(
    # EXPORT ${MODULE_EXPORT_GROUP}
    # DESTINATION "lib/cmake"
# )
