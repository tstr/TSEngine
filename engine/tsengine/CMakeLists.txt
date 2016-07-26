
SET(src_list
	../tsengine.h
	engine.cpp
)

ADD_LIBRARY(
	tsengine STATIC
	${src_list}
)


TARGET_LINK_LIBRARIES(tsengine tscore)
