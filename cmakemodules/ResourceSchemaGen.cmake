################################################################################################
#
#	Resource Schema Generator cmake module
#
################################################################################################

set(RCS_GEN_TARGET rcschema)
set(RCS_GEN_EXECUTABLE "$<TARGET_FILE:${RCS_GEN_TARGET}>")
set(RCS_GEN_LIBRARIES rcschemaLib)
set(RCS_GEN_EXT "rcs.h")

#
#	Generate C++ header
#
#	SCHEMAS		- List of schema files
#	HEADER_VAR	- Output variable to store list of generated headers
#	HEADER_DIR	- Optional directory to store generated headers in, if relative path it is assumed to be relative to binary directory
#
function(generate_cpp_rschemas)

	set(options APPEND)
	set(args HEADER_VAR HEADER_DIR)
	set(argsMulti SCHEMAS)
	cmake_parse_arguments(param "${options}" "${args}" "${argsMulti}" ${ARGN})
	
	# Clear header list
	set(${param_HEADER_VAR})

	# Default output prefix is the current binary directory
	set(_prefix_path "${CMAKE_CURRENT_BINARY_DIR}")
	
	# If header output directory is given
	if (NOT param_HEADER_DIR STREQUAL "")
		
		# If header output directory is an absolute path
		if (IS_ABSOLUTE "${param_HEADER_DIR}")
			# Set prefix to header output
			set(_prefix_path "${param_HEADER_DIR}")
		else()
			# Otherwise append header output directory to prefix
			set(_prefix_path "${CMAKE_CURRENT_BINARY_DIR}/${param_HEADER_DIR}")
		endif()
		
		file(MAKE_DIRECTORY "${_prefix_path}")
		
	endif()
	
	# For each schema file
	foreach (schema ${param_SCHEMAS})
		
		get_filename_component(_schema_path ${schema} ABSOLUTE)
		get_filename_component(_schema_name ${schema} NAME_WE)
		
		set(schema_output "${_prefix_path}/${_schema_name}.${RCS_GEN_EXT}")
		get_filename_component(schema_output ${schema_output} ABSOLUTE)
		
		add_custom_command(
			OUTPUT "${schema_output}"
			COMMAND ${RCS_GEN_EXECUTABLE}
			ARGS --out ${_prefix_path} ${_schema_path}
			MAIN_DEPENDENCY ${_schema_path}
			DEPENDS ${RCS_GEN_TARGET}
			VERBATIM
		)
		
		list(APPEND ${param_HEADER_VAR} ${schema_output})
		
	endforeach()
	
	# Update generated property
	set_source_files_properties(${${param_HEADER_VAR}} PROPERTIES GENERATED TRUE)
	set(${param_HEADER_VAR} ${${param_HEADER_VAR}} PARENT_SCOPE)
	
endfunction()


#
#	Generate a set of schema headers for a given target
#
#	SCHEMAS		- List of schema files
#	TARGET		- Name of target to attach generated headers to
#	HEADER_VAR	- Output variable to store list of generated headers
#	APPENDPATH	- Append directory of schema to header path output
#
function(target_cpp_rschemas)

	set(options APPENDPATH)
	set(args TARGET HEADER_VAR)
	set(argsMulti SCHEMAS)
	cmake_parse_arguments(param "${options}" "${args}" "${argsMulti}" ${ARGN})
	
	set(out_dir "${CMAKE_CURRENT_BINARY_DIR}")
	set(_schema_headers)
	
	foreach(_schema_file ${param_SCHEMAS})
		
		set(_append_dir "/")
		
		#If path of schema should be appended to output path of header
		if (param_APPENDPATH)
			get_filename_component(_schema_dir ${_schema_file} DIRECTORY)
			
			if (NOT IS_ABSOLUTE "${_schema_dir}")
				set(_append_dir "/${_schema_dir}")
			endif()
		endif()
		
		set(_header)
		
		# Generate headers
		generate_cpp_rschemas(HEADER_VAR _header HEADER_DIR "${out_dir}${_append_dir}" SCHEMAS ${_schema_file} ${_append})
		set(_schema_headers ${_header} ${_schema_headers})
		
	endforeach()
	
	target_sources(${param_TARGET} PRIVATE ${_schema_headers})
	target_sources(${param_TARGET} PRIVATE ${param_SCHEMAS})
	
	target_include_directories(${param_TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
	target_link_libraries(${param_TARGET} PUBLIC ${RCS_GEN_LIBRARIES})
	
	set(${param_HEADER_VAR})
	set(${param_HEADER_VAR} ${_schema_headers} PARENT_SCOPE)
	
endfunction()

################################################################################################
