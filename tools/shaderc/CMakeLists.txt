#
#	ShaderC:
#	Tool for compiling shaders
#

#####################################################################################################################
#	Shader Compiler project
#####################################################################################################################

SET (shaderc_src
	
	#hash functions
	src/crypto/sha256.h
	src/crypto/sha256.c
	src/crypto/md5.h
	src/crypto/md5.c

	src/backend/backend.h
	src/backend/hlsl.h
	src/backend/hlsl.cpp
	
	src/frontend/tree.h
	src/frontend/parser.h
	src/frontend/parser.cpp
	
	src/common.h
	src/compiler.h
	src/compiler.cpp
	src/preprocessor.h
	src/preprocessor.cpp
	src/main.cpp
)

ADD_EXECUTABLE(
	shaderc
	${shaderc_src}
)

INSTALL(
	TARGETS shaderc
	RUNTIME DESTINATION ${TS_BINARY_INSTALL}
	LIBRARY DESTINATION ${TS_BINARY_INSTALL}
	ARCHIVE DESTINATION ${TS_LIBRARY_INSTALL}
)

assign_source_groups(${shaderc_src})

TARGET_LINK_LIBRARIES(
	shaderc PRIVATE
	tscore
)

#####################################################################################################################

set(test_src
	test/shaders/shader.vs
	test/shaders/shader.ps
	test/manifest.shm
	test/TestCompiler.cpp
)

ADD_EXECUTABLE(
	TestShaderc
	${test_src}
)

TARGET_LINK_LIBRARIES(
	TestShaderc PRIVATE
	tscore
	tsgraphics
)

assign_source_groups(${test_src})

ADD_DEPENDENCIES(TestShaderc shaderc)

ADD_TEST(
	NAME TestShaderc
	COMMAND $<TARGET_FILE:TestShaderc> "$<TARGET_FILE:shaderc>" "${CMAKE_CURRENT_SOURCE_DIR}/test/" "${CMAKE_CURRENT_BINARY_DIR}/test/"
	#COMMAND $<TARGET_FILE:shaderc> -t "${CMAKE_CURRENT_SOURCE_DIR}/test/manifest.shm" -s "${CMAKE_CURRENT_SOURCE_DIR}/test/shaders/" -o "${CMAKE_CURRENT_BINARY_DIR}/test/bin/" && $<TARGET_FILE:TestShaderc> "${CMAKE_CURRENT_BINARY_DIR}/test/bin/"
)

#####################################################################################################################
