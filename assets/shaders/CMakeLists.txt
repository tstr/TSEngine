#
#	Shaders
#

set(shadercomp_sources
	src/rendertest/testshader.fx
)

set(shadercomp_manifests
	src/rendertest.shm
)

set(shadercomp_stamps "")

foreach(f ${shadercomp_sources} ${shadercomp_manifests})
	
	list(APPEND shadercomp_stamps "${CMAKE_CURRENT_BINARY_DIR}/${f}.stamp")
	
endforeach()

add_custom_target(
	PrecompileShaders ALL
	SOURCES ${shadercomp_sources} ${shadercomp_manifests}
	DEPENDS ${shadercomp_stamps}
)

add_dependencies(PrecompileShaders shaderc)

assign_source_groups(${shadercomp_sources} ${shadercomp_manifests})

foreach(f ${shadercomp_manifests})

	add_custom_command(
		OUTPUT ${shadercomp_stamps}
		COMMAND $<TARGET_FILE:shaderc> -t "${CMAKE_CURRENT_SOURCE_DIR}/${f}" -s "${CMAKE_CURRENT_SOURCE_DIR}/src/" -o "${CMAKE_CURRENT_SOURCE_DIR}/bin/"
		DEPENDS ${shadercomp_sources} ${f}
	)

endforeach()
