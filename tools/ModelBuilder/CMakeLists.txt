#
#	Tool for converting model files
#

#####################################################################################################################
# Assimp external project
#####################################################################################################################

INCLUDE(ExternalProject)

SET(assimp_prefix ${CMAKE_CURRENT_BINARY_DIR}/external/assimp)

SET(assimp_cmake_args
	${TS_EXTERNAL_CMAKE_ARGS}
	-DASSIMP_BUILD_ASSIMP_TOOLS:BOOL=OFF
	-DASSIMP_BUILD_ASSIMP_VIEW:BOOL=OFF
	-DASSIMP_BUILD_TESTS:BOOL=OFF
	-DBUILD_SHARED_LIBS:BOOL=OFF
	-DLIBRARY_SUFFIX:STRING=
)

ExternalProject_Add(
	assimpproj
	PREFIX ${assimp_prefix}
	GIT_REPOSITORY https://github.com/assimp/assimp.git
	GIT_TAG master
	UPDATE_COMMAND ""
	#PATCH_COMMAND ""
	#INSTALL_COMMAND ""
	
	CMAKE_ARGS ${assimp_cmake_args}
)

ExternalProject_Get_Property(assimpproj INSTALL_DIR)

# FIND_PACKAGE(assimp CONFIG PATHS ${INSTALL_DIR})
# MARK_AS_ADVANCED(assimp_DIR)

#message("Assimp config dir: ${assimp_DIR}")
#message("Assimp install dir: ${INSTALL_DIR}")

INCLUDE_DIRECTORIES(${INSTALL_DIR}/include)
# target_include_directories(assimp PUBLIC ${INSTALL_DIR}/include)

ADD_LIBRARY(assimp STATIC IMPORTED)
SET_PROPERTY(TARGET assimp PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib/assimp${CMAKE_STATIC_LIBRARY_SUFFIX})
ADD_LIBRARY(zlibstatic STATIC IMPORTED)
SET_PROPERTY(TARGET zlibstatic PROPERTY IMPORTED_LOCATION_DEBUG ${INSTALL_DIR}/lib/zlibstaticd${CMAKE_STATIC_LIBRARY_SUFFIX})
SET_PROPERTY(TARGET zlibstatic PROPERTY IMPORTED_LOCATION_RELEASE ${INSTALL_DIR}/lib/zlibstatic${CMAKE_STATIC_LIBRARY_SUFFIX})
ADD_DEPENDENCIES(assimp assimpproj zlibstatic)

#####################################################################################################################
# Model builder project
#####################################################################################################################

SET (ModelBuilder_list
	build.cpp
)

ADD_EXECUTABLE(modelbuilder ${ModelBuilder_list})

TARGET_LINK_LIBRARIES(modelbuilder
	tsengine
	assimp
	zlibstatic
)

#FIND_LIBRARY(ASSIMP NAMES "assimp" PATHS ${PROJECT_SOURCE_DIR}/dependencies/)
#TARGET_LINK_LIBRARIES(modelbuilder ASSIMP)

#####################################################################################################################

# ADD_CUSTOM_TARGET(modelbuilder_importer ALL)

# ADD_CUSTOM_COMMAND(
	# TARGET modelbuilder POST_BUILD
	# COMMAND "$<TARGET_FILE_DIR:modelbuilder>/modelbuilder.exe" -t = "${PROJECT_SOURCE_DIR}/cube.obj" -o = "${PROJECT_BINARY_DIR}"
# )

# SET_TARGET_PROPERTIES(modelbuilder_importer PROPERTIES FOLDER tools)

#####################################################################################################################
