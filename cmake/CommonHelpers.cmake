################################################################################################
#
#	Common set of helper functions/modules
#
################################################################################################

#
#   Create source groups based on the directory structure
#
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

#
#	Install pdb files
#
function(install_target_pdb _target)
    
	# Verify target name
	if (NOT TARGET ${_target})
		message(CRITICAL_ERROR "${_target} is not a valid target")
	endif()
	
	# If target is static library install to lib
	# otherwise install to bin
	set(_destination "bin")
	
	get_target_property(_target_type ${_target} TYPE)
	
	if (_target_type STREQUAL "STATIC_LIBRARY")
	  set(_destination "lib")
	endif ()
	
	# Install PDB if it exists
	install(
		FILES "$<TARGET_FILE_DIR:${_target}>/${_target}.pdb"
		DESTINATION ${_destination}
		CONFIGURATIONS Debug
	)
	
endfunction()
