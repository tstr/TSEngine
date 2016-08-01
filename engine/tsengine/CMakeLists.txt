#TSENGINE

SET(src_list
	../tsengine.h
	tsengine.cpp
	cmdargs.cpp
	cmdargs.h
)

ADD_LIBRARY(
	tsengine STATIC
	${src_list}
)


TARGET_LINK_LIBRARIES(tsengine tscore)

copy_source_groups(${src_list})