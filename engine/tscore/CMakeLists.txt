#TSCORE

SET(src_list
	platform/console.h
	platform/console.cpp
	platform/window.h
	platform/window.cpp
	debug/assert.h
	debug/assert.cpp
	debug/log.h
	debug/log.cpp
	types.h
)

ADD_LIBRARY(
	tscore STATIC
	${src_list}
)

copy_source_groups(${src_list})