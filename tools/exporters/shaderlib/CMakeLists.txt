#
#	ShaderLib:
#	Tool for compiling shaders
#

set(shaderc_inc
	include/Preprocessor.h
	include/ShaderParser.h
	include/ShaderType.h
	include/ShaderAnnotation.h
	include/ShaderCommon.h
	include/ShaderCompiler.h
    include/ShaderSource.h
)

SET (shaderc_src
	#hash functions
	src/crypto/sha256.h
	src/crypto/sha256.c
	src/crypto/md5.h
	src/crypto/md5.c

	src/backend/backend.h
	src/backend/hlsl.h
	src/backend/hlsl.cpp
	
    src/util/Scanner.h
    src/util/Scanner.cpp
    src/util/Token.h
    src/util/CharTypes.h
    
	src/Preprocessor.cpp
	src/ShaderCompiler.cpp
    src/ShaderParser.cpp
	src/ShaderType.cpp
    src/ShaderSource.cpp
	
	#python bindings
	binding.cpp
)

add_pybind_module(shaderlib ${shaderc_inc} ${shaderc_src})

target_link_libraries(shaderlib PUBLIC tsgraphics)
target_include_directories(shaderlib PUBLIC "include")

#####################################################################################################################
