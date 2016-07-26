#TSCORE.lib

SET(src_list
	platform/window.h
	platform/window.cpp
	assert.h
	assert.cpp
	types.h
)

ADD_LIBRARY(
	tscore STATIC
	${src_list}
)
