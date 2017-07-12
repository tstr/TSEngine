################################################################################################
#
#	Samples
#
################################################################################################

set(SAMPLES_FOLDER samples)

#Add a component executable
function(add_sample)
	
	set(options)
	set(args NAME SHADER_MANIFEST CONFIG)
	set(argsMulti SOURCES SHADERS)
	cmake_parse_arguments(add_sample "${options}" "${args}" "${argsMulti}" ${ARGN})
	
	ADD_EXECUTABLE(
		${add_sample_NAME}
		${add_sample_SOURCES} ${add_sample_SHADERS} ${add_sample_SHADER_MANIFEST} ${add_sample_CONFIG}
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
#	Sample applications
################################################################################################

ADD_SUBDIRECTORY(Sandbox)
ADD_SUBDIRECTORY(SampleTess)

################################################################################################
#	Automatic shader compilation
################################################################################################

ADD_DEPENDENCIES(Sandbox ${SHADER_GENERATOR_TARGET})
ADD_DEPENDENCIES(SampleTess ${SHADER_GENERATOR_TARGET})

InitShaderGenerator()

################################################################################################
