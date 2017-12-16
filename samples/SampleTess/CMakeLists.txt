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

ADD_SAMPLE(
	NAME SampleTess
	SOURCES ${src_list}
	SHADERS ${shader_src_list}
	CONFIG config.ini
)

ASSIGN_SOURCE_GROUPS(${src_list} ${shader_src_list})

TARGET_LINK_LIBRARIES(SampleTess PUBLIC tsengine)
