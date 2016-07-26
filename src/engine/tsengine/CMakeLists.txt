
SET(src_list

)

ADD_LIBRARY(
	tsengine STATIC
	${src_list}
)


TARGET_LINK_LIBRARIES(tsengine tscore)
