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
	
	src/frontend/importer.h
	src/frontend/importer.cpp
	
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

target_link_libraries(
	shaderc PRIVATE
	tsgraphics
)

#####################################################################################################################
