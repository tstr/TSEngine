#####################################################################################################################
#	Assimp external project module
#####################################################################################################################

ExternalProject_Add(
	ep_assimp
	PREFIX ${EP_PREFIX}
	GIT_REPOSITORY https://github.com/assimp/assimp.git
	GIT_TAG "v4.1.0"
	
	INSTALL_DIR ${EP_STAGE_DIR}
	UPDATE_COMMAND ""
	
	CMAKE_ARGS
    	-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> 
        -DASSIMP_BUILD_ASSIMP_TOOLS:BOOL=OFF
        -DASSIMP_BUILD_ASSIMP_VIEW:BOOL=OFF
        -DASSIMP_BUILD_TESTS:BOOL=OFF
        -DBUILD_SHARED_LIBS:BOOL=ON
        -DLIBRARY_SUFFIX:STRING=
)

SET_TARGET_PROPERTIES(ep_assimp PROPERTIES FOLDER external)

#####################################################################################################################
# Set up imported targets manually
#####################################################################################################################

ExternalProject_Get_Property(ep_assimp INSTALL_DIR)

# Ensure install directories exist at configure time
file(MAKE_DIRECTORY "${INSTALL_DIR}/include")
file(MAKE_DIRECTORY "${INSTALL_DIR}/lib")

#
# zlib
#
add_library(assimp::zlib STATIC IMPORTED)

set_target_properties(assimp::zlib PROPERTIES
	IMPORTED_LOCATION_DEBUG "${INSTALL_DIR}/lib/zlibstaticd${CMAKE_STATIC_LIBRARY_SUFFIX}"
	IMPORTED_LOCATION_RELEASE "${INSTALL_DIR}/lib/zlibstatic${CMAKE_STATIC_LIBRARY_SUFFIX}"
	IMPORTED_LOCATION_RELWITHDEBINFO "${INSTALL_DIR}/lib/zlibstatic${CMAKE_STATIC_LIBRARY_SUFFIX}"
	IMPORTED_LOCATION_MINSIZEREL "${INSTALL_DIR}/lib/zlibstatic${CMAKE_STATIC_LIBRARY_SUFFIX}"
)

#
# assimp
#
add_library(assimp SHARED IMPORTED)

set_target_properties(assimp PROPERTIES
	# Shared library location
	IMPORTED_LOCATION "${INSTALL_DIR}/bin/assimp${CMAKE_SHARED_LIBRARY_SUFFIX}"
	IMPORTED_IMPLIB "${INSTALL_DIR}/lib/assimp${CMAKE_STATIC_LIBRARY_SUFFIX}"
	# Dependencies
	IMPORTED_LINK_INTERFACE_LIBRARIES assimp::zlib
	# Include directories
	INTERFACE_INCLUDE_DIRECTORIES "$<BUILD_INTERFACE:${INSTALL_DIR}/include>"
)

# Copy assimp dll to project output
add_custom_command(
	TARGET ep_assimp POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${INSTALL_DIR}/bin/assimp${CMAKE_SHARED_LIBRARY_SUFFIX}"
		"${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/$<CONFIG>"
)

# Copy pdb file to project output
add_custom_command(
	TARGET ep_assimp POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${INSTALL_DIR}/lib/assimp.pdb"
		"${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/$<CONFIG>"
)

# Imported library depends on external project
add_dependencies(assimp ep_assimp)

#####################################################################################################################
