#
#	Shaders
#

set(shadercomp_sources
	src/rendertest/cube.fx
	src/rendertest/quad.fx
)

set(shadercomp_manifests
	src/rendertest.shm
)

set(shadercomp_stamps "")

foreach(m ${shadercomp_manifests})# ${shadercomp_sources})
	
	list(APPEND shadercomp_stamps "${CMAKE_CURRENT_BINARY_DIR}/${m}.stamp")
	
endforeach()

add_custom_target(
	PrecompileShaders ALL
	SOURCES ${shadercomp_sources} ${shadercomp_manifests}
	DEPENDS ${shadercomp_stamps}
)

add_dependencies(PrecompileShaders shaderc)

assign_source_groups(${shadercomp_sources} ${shadercomp_manifests})

#Foreach manifest
foreach(m ${shadercomp_manifests})

	set(manifest_stamp "${CMAKE_CURRENT_BINARY_DIR}/${m}.stamp")
	
	add_custom_command(
		OUTPUT ${manifest_stamp}					# stamp file
		COMMAND cmake -E touch "${manifest_stamp}"	# update stamp file
		COMMAND $<TARGET_FILE:shaderc> -t "${CMAKE_CURRENT_SOURCE_DIR}/${m}" -s "${CMAKE_CURRENT_SOURCE_DIR}/src/" -o "${CMAKE_CURRENT_SOURCE_DIR}/bin/"
		DEPENDS ${m} ${shadercomp_sources} 			# when either the manifest or sources are modified run command
	)

endforeach()
