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
	debug/profiling.h

	system/memory.h
	system/thread.h
	system/time.h
	system/info.h
	system/info.cpp
	
	io/stream.h
	
	maths/common.h
	maths/functions.h
	maths/maths.cpp
	maths/matrix.h
	maths/quaternion.h
	maths/vector.h
	maths.h
	
	types.h
	strings.h
	cmdargs.h
	cmdargs.cpp
)

ADD_LIBRARY(
	tscore STATIC
	${src_list}
)

copy_source_groups(${src_list})