#
#	SampleTess
#

SET(src_list
	
	src/main.cpp
)

SET(shader_src_list
	
	shaders/shader.fx
	shaders/quad.fx
)

SET(shader_manifest shaders/manifest.shm)

ADD_SAMPLE(
	NAME SampleTess
	SOURCES ${src_list}
	SHADERS ${shader_src_list}
	SHADER_MANIFEST ${shader_manifest}
	CONFIG config.ini
)

ASSIGN_SOURCE_GROUPS(${src_list} ${shader_src_list} ${shader_manifest})

TARGET_LINK_LIBRARIES(SampleTess PUBLIC tsengine)
