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
	
	src/Common.h
	src/ShaderCompiler.h
	src/ShaderCompiler.cpp

    src/util/Scanner.h
    src/util/Scanner.cpp
    src/util/Token.h
    src/util/CharTypes.h
    
	src/Preprocessor.h
	src/Preprocessor.cpp
    src/ShaderParser.h
    src/ShaderParser.cpp
    src/ShaderType.h
	src/ShaderType.cpp
	src/ShaderAnnotation.h
	
	src/main.cpp
)

ADD_EXECUTABLE(
	shaderc
	${shaderc_src}
)

install_tools(shaderc)

assign_source_groups(${shaderc_src})

TARGET_LINK_LIBRARIES(
	shaderc PRIVATE
	tsgraphics
    CLIutil
)

#####################################################################################################################
# Testing
#####################################################################################################################

if(TS_BUILD_TESTS)

set(test_src
	test/shaders/shader.s
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
	COMMAND $<TARGET_FILE:TestShaderc> "$<TARGET_FILE:shaderc>" "${CMAKE_CURRENT_SOURCE_DIR}/test/shaders" "${CMAKE_CURRENT_BINARY_DIR}/test/shaders"
	#COMMAND $<TARGET_FILE:shaderc> -t "${CMAKE_CURRENT_SOURCE_DIR}/test/manifest.shm" -s "${CMAKE_CURRENT_SOURCE_DIR}/test/shaders/" -o "${CMAKE_CURRENT_BINARY_DIR}/test/bin/" && $<TARGET_FILE:TestShaderc> "${CMAKE_CURRENT_BINARY_DIR}/test/bin/"
)

SET_TARGET_PROPERTIES(
	TestShaderc
	PROPERTIES FOLDER tools/tests
)

endif()

#####################################################################################################################
