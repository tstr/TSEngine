################################################################################################
#
#	Engine Libraries
#
################################################################################################

include(GenerateExportHeader)

SET(TS_BUILD_SHARED_LIBRARIES CACHE BOOL OFF)

set(ENGINE_FOLDER )

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
	
	SET_TARGET_PROPERTIES(
		${libname}
		PROPERTIES FOLDER engine
	)
	
endfunction()

################################################################################################
#	engine components
################################################################################################

ADD_SUBDIRECTORY(tscore)
ADD_SUBDIRECTORY(tsengine)
ADD_SUBDIRECTORY(tsgraphics)

################################################################################################
