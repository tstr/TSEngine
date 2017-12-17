################################################################################################
#	Automatic shader generator
################################################################################################

# Global variables
set (SHADER_COMPILER_LOG_FILE "${PROJECT_BINARY_DIR}/shadergen/out.log")
set (SHADER_COMPILER_OUTPUT_DIR "${PROJECT_SOURCE_DIR}/assets/shaderbin/")
set (SHADER_COMPILER_COMMAND "$<TARGET_FILE:shaderc>")
set (SHADER_COMPILER_TARGET shaderc)

file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/shadergen/")

#
#	Set up automatic recompilation of shaders
#
#	SHADER_VAR	- Output variable of generated shader objects
#	OUTPUT_DIR	- Name of target to attach generated headers to
#	SOURCES	- Input shader sources
#
function(compile_shaders)

	set(options)
	set(args OUTPUT_DIR)
	set(argsMulti SHADER_VAR SOURCES)
	cmake_parse_arguments(param "${options}" "${args}" "${argsMulti}" ${ARGN})
	
	set(_out_dir "${SHADER_COMPILER_OUTPUT_DIR}/${param_OUTPUT_DIR}")
	
	set(${param_SHADER_VAR})
	
	foreach(shader_source ${param_SOURCES})
		
		get_filename_component(shader_object ${shader_source} NAME_WE)
		
		set(shader_object "${_out_dir}/${shader_object}.tsh")
		
		get_filename_component(shader_source ${shader_source} ABSOLUTE)
		
		add_custom_command(
			OUTPUT ${shader_object}
			COMMAND ${SHADER_COMPILER_COMMAND} --out "${_out_dir}" ${shader_source} >> "${SHADER_COMPILER_LOG_FILE}"
			DEPENDS ${SHADER_COMPILER_TARGET} ${param_SOURCES}
			COMMENT "Compiling shader ${shader_source}"
			VERBATIM
		)
		
		list(APPEND ${param_SHADER_VAR} ${shader_object})
		
	endforeach()
	
	set(${param_SHADER_VAR} ${${param_SHADER_VAR}} PARENT_SCOPE)
	
endfunction()

#
#	Set up automatic recompilation of shaders for a given target
#
#	TARGET		- Target to attach dependency to
#	OUTPUT_DIR	- Name of target to attach generated headers to
#	SOURCES		- Input shader sources
#
function(target_compile_shaders)

	set(options)
	set(args TARGET OUTPUT_DIR)
	set(argsMulti SOURCES)
	cmake_parse_arguments(param "${options}" "${args}" "${argsMulti}" ${ARGN})
	
	compile_shaders(SHADER_VAR _shader_var OUTPUT_DIR ${param_OUTPUT_DIR} SOURCES ${param_SOURCES})
	
	target_sources(${param_TARGET} PRIVATE ${_shader_var})
	target_sources(${param_TARGET} PRIVATE ${param_SOURCES})

	foreach (f ${_shader_var} ${param_SOURCES})
	
		get_filename_component(filepath "${f}" ABSOLUTE)
		file(TO_NATIVE_PATH "${filepath}" filepath)
		source_group("shaders" FILES "${filepath}")
		
	endforeach()
	
	add_dependencies(${param_TARGET} ${SHADER_COMPILER_TARGET})
	
endfunction()
