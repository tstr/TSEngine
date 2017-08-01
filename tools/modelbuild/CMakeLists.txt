#
#	ModelBuild:
#	Tool for converting model files
#

#####################################################################################################################
#	Model builder project
#####################################################################################################################

SET (modelbuild_src
	src/build.cpp
)

ADD_EXECUTABLE(
	modelbuild
	${modelbuild_src}
)

install_tools(modelbuild)

assign_source_groups(${modelbuild_src})

target_link_libraries(
	modelbuild PRIVATE
	tscore
	tsgraphics
	assimpLib
	zlib
)

#####################################################################################################################
#	Assimp external project
#####################################################################################################################

INCLUDE(ExternalProject)

SET(assimp_prefix "${PROJECT_BINARY_DIR}/external/assimp")

SET(assimp_cmake_args
	-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> 
	-DASSIMP_BUILD_ASSIMP_TOOLS:BOOL=OFF
	-DASSIMP_BUILD_ASSIMP_VIEW:BOOL=OFF
	-DASSIMP_BUILD_TESTS:BOOL=OFF
	-DBUILD_SHARED_LIBS:BOOL=OFF
	-DLIBRARY_SUFFIX:STRING=
)

ExternalProject_Add(
	assimp
	PREFIX ${assimp_prefix}
	GIT_REPOSITORY https://github.com/assimp/assimp.git
	GIT_TAG master
	UPDATE_COMMAND ""
	#PATCH_COMMAND ""
	#INSTALL_COMMAND ""
	
	CMAKE_ARGS ${assimp_cmake_args}
)

ExternalProject_Get_Property(assimp BINARY_DIR)
ExternalProject_Get_Property(assimp INSTALL_DIR)

add_dependencies(modelbuild assimp)

#assimp
add_library(assimpLib STATIC IMPORTED)
set_target_properties(assimpLib PROPERTIES IMPORTED_LOCATION_DEBUG "${BINARY_DIR}/code/Debug/assimp${CMAKE_STATIC_LIBRARY_SUFFIX}")
set_target_properties(assimpLib PROPERTIES IMPORTED_LOCATION_RELEASE "${BINARY_DIR}/code/Release/assimp${CMAKE_STATIC_LIBRARY_SUFFIX}")
set_target_properties(assimpLib PROPERTIES IMPORTED_LOCATION_RELWITHDEBINFO "${BINARY_DIR}/code/RelWithDebInfo/assimp${CMAKE_STATIC_LIBRARY_SUFFIX}")
set_target_properties(assimpLib PROPERTIES IMPORTED_LOCATION_MINSIZEREL "${BINARY_DIR}/code/MinSizeRel/assimp${CMAKE_STATIC_LIBRARY_SUFFIX}")

#zlib
add_library(zlib STATIC IMPORTED)
set_target_properties(zlib PROPERTIES IMPORTED_LOCATION_DEBUG "${BINARY_DIR}/contrib/zlib/Debug/zlibstaticd${CMAKE_STATIC_LIBRARY_SUFFIX}")
set_target_properties(zlib PROPERTIES IMPORTED_LOCATION_RELEASE "${BINARY_DIR}/contrib/zlib/Release/zlibstatic${CMAKE_STATIC_LIBRARY_SUFFIX}")
set_target_properties(zlib PROPERTIES IMPORTED_LOCATION_RELWITHDEBINFO "${BINARY_DIR}/contrib/zlib/RelWithDebInfo/zlibstatic${CMAKE_STATIC_LIBRARY_SUFFIX}")
set_target_properties(zlib PROPERTIES IMPORTED_LOCATION_MINSIZEREL "${BINARY_DIR}/contrib/zlib/MinSizeRel/zlibstatic${CMAKE_STATIC_LIBRARY_SUFFIX}")

#External project include directories
target_include_directories(modelbuild PRIVATE "${INSTALL_DIR}/include")

#####################################################################################################################

# ADD_CUSTOM_TARGET(modelbuild_importer ALL)

# ADD_CUSTOM_COMMAND(
	# TARGET modelbuild POST_BUILD
	# COMMAND "$<TARGET_FILE_DIR:modelbuild>/modelbuild.exe" -t = "${PROJECT_SOURCE_DIR}/cube.obj" -o = "${PROJECT_BINARY_DIR}"
# )

# SET_TARGET_PROPERTIES(modelbuild_importer PROPERTIES FOLDER tools)

#####################################################################################################################
