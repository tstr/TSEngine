#
#	ShaderLib:
#	Tool for compiling shaders
#

#####################################################################################################################
#	Shader Compiler sources
#####################################################################################################################

include(PythonUtils)

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
	
	#python bindings
	binding.cpp
)

add_pybind_module(shaderlib ${shaderc_src})

target_link_libraries(
	shaderlib PRIVATE
	tsgraphics
)

add_dependencies(DATA_BUILD shaderlib)

set_target_properties(shaderlib PROPERTIES FOLDER tools/extensions)

assign_source_groups(${shaderc_src})

#####################################################################################################################
