################################################################################################
#
#	Samples
#
################################################################################################

include(CommonHelpers)

set(SAMPLES_FOLDER samples)

#Add a component executable
function(add_sample)
	
	set(options)
	set(args NAME CONFIG)
	set(argsMulti SOURCES SHADERS)
	cmake_parse_arguments(ARG "${options}" "${args}" "${argsMulti}" ${ARGN})
	
	ADD_EXECUTABLE(
		${ARG_NAME}
		${ARG_SOURCES} ${ARG_CONFIG}
	)
	
	install_target_pdb(${ARG_NAME})
	
	INSTALL(
		TARGETS ${ARG_NAME}
		RUNTIME DESTINATION ${TS_BINARY_INSTALL}
		LIBRARY DESTINATION ${TS_BINARY_INSTALL}
		ARCHIVE DESTINATION ${TS_LIBRARY_INSTALL}
	)
	
	SET_TARGET_PROPERTIES(
		${ARG_NAME}
		PROPERTIES FOLDER ${SAMPLES_FOLDER}
	)
	
	CONFIGURE_FILE(
		"${CMAKE_CURRENT_SOURCE_DIR}/${ARG_CONFIG}"
		"${CMAKE_CURRENT_BINARY_DIR}/${ARG_CONFIG}"
	)
	
	INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/${ARG_CONFIG}" DESTINATION ${TS_BINARY_INSTALL})
	
endfunction()

################################################################################################
#	Sample applications
################################################################################################

ADD_SUBDIRECTORY(Sandbox)
#ADD_SUBDIRECTORY(SampleTess)

################################################################################################
