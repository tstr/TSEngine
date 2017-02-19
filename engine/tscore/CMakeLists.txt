#
#	tscore
#

set(tscore_inc
	
	inc/tscore/abi.h
	
	inc/tscore/debug/assert.h
	inc/tscore/debug/profiling.h
	inc/tscore/debug/log.h

	inc/tscore/system/memory.h
	inc/tscore/system/thread.h
	inc/tscore/system/error.h
	inc/tscore/system/time.h
	inc/tscore/system/info.h
	
	inc/tscore/filesystem/path.h
	inc/tscore/filesystem/pathhelpers.h
	
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
)

SET(tscore_src
	
	src/maths.cpp
	
	src/assert.cpp
	src/log.cpp

	src/system/memory.cpp
	src/system/error.cpp
	src/system/info.cpp

	src/filesystem/path.cpp
	src/filesystem/pathhelpers.cpp
)

SET(tscore_files ${tscore_src} ${tscore_inc})

add_library_tse(
	tscore
	"${tscore_files}"
)

target_include_directories(
	tscore
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

assign_source_groups(${tscore_files})
