#####################################################################################
#
#	tscore
#
#	 Core utility module
#
#####################################################################################

set(tscore_inc
	
	inc/tscore/abi.h
	
	inc/tscore/debug/assert.h
	inc/tscore/debug/profiling.h
	inc/tscore/debug/log.h

	inc/tscore/alloc/Linear.h

	inc/tscore/system/memory.h
	inc/tscore/system/thread.h
	inc/tscore/system/time.h
	
	inc/tscore/path.h
	inc/tscore/pathutil.h
	
	inc/tscore/containers/circularbuffer.h
	inc/tscore/containers/threadqueue.h
	inc/tscore/containers/stack.h
	
	inc/tscore/maths/common.h
	inc/tscore/maths/functions.h
	inc/tscore/maths/matrix.h
	inc/tscore/maths/quaternion.h
	inc/tscore/maths/vector.h
	inc/tscore/maths.h
	
	inc/tscore/types.h
	inc/tscore/strings.h
	inc/tscore/delegate.h
	inc/tscore/ptr.h
	inc/tscore/table.h
	inc/tscore/signal.h
)

SET(tscore_src
	
	src/maths.cpp
	src/assert.cpp
	src/log.cpp
	src/memory.cpp
	src/path.cpp
	src/pathutil.cpp
)

add_engine_module(
	NAME tscore
	SOURCES ${tscore_src}
	HEADERS ${tscore_inc}
	PUBLIC_DIR inc
	PRIVATE_DIR src
)

#####################################################################################
