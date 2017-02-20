#
#	RenderTest
#

SET(src_list
	
	src/test.cpp
)

ADD_SAMPLE(
	rendertest
	"${src_list}"
)

ASSIGN_SOURCE_GROUPS(${src_list})

TARGET_LINK_LIBRARIES(rendertest PUBLIC tsengine)
