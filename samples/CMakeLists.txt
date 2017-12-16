################################################################################################
#
#	Samples
#
################################################################################################

include(ShaderCompiler)

set(SAMPLES_FOLDER samples)

#Add a component executable
function(add_sample)
	
	set(options)
	set(args NAME CONFIG)
	set(argsMulti SOURCES SHADERS)
	cmake_parse_arguments(add_sample "${options}" "${args}" "${argsMulti}" ${ARGN})
	
	ADD_EXECUTABLE(
		${add_sample_NAME}
		${add_sample_SOURCES} ${add_sample_CONFIG}
	)
	
	target_compile_shaders(TARGET ${add_sample_NAME} SOURCES ${add_sample_SHADERS})
	
	INSTALL(
		TARGETS ${add_sample_NAME}
		RUNTIME DESTINATION ${TS_BINARY_INSTALL}
		LIBRARY DESTINATION ${TS_BINARY_INSTALL}
		ARCHIVE DESTINATION ${TS_LIBRARY_INSTALL}
	)
	
	SET_TARGET_PROPERTIES(
		${add_sample_NAME}
		PROPERTIES FOLDER ${SAMPLES_FOLDER}
	)
	
	CONFIGURE_FILE(
		"${CMAKE_CURRENT_SOURCE_DIR}/${add_sample_CONFIG}"
		"${CMAKE_CURRENT_BINARY_DIR}/${add_sample_CONFIG}"
	)
	
endfunction()

################################################################################################
#	Sample applications
################################################################################################

ADD_SUBDIRECTORY(Sandbox)
#ADD_SUBDIRECTORY(SampleTess)

################################################################################################
