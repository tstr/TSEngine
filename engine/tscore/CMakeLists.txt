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
	
	system/memory.h
	
	maths/colour.h
	maths/functions.h
	maths/maths.cpp
	maths/mathsutils.h
	maths/matrix.h
	maths/quaternion.h
	maths/vector.h
	
	types.h
)

ADD_LIBRARY(
	tscore STATIC
	${src_list}
)

copy_source_groups(${src_list})