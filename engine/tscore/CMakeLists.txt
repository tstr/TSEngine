#TSCORE

SET(src_list

	internal/tscoreapi.h
	
	debug/assert.h
	debug/assert.cpp
	debug/log.h
	debug/log.cpp
	debug/profiling.h

	system/memory.h
	system/memory.cpp
	system/thread.h
	system/error.h
	system/error.cpp
	system/time.h
	system/info.h
	system/info.cpp

	filesystem/path.cpp
	filesystem/path.h

	containers/circularbuffer.h
	containers/threadqueue.h
	containers/stack.h
	
	maths/common.h
	maths/functions.h
	maths/maths.cpp
	maths/matrix.h
	maths/quaternion.h
	maths/vector.h
	maths.h
	
	types.h
	strings.h
	delegate.h
)

ADD_LIBRARY_EX(
	tscore
	"${src_list}"
)

copy_source_groups(${src_list})