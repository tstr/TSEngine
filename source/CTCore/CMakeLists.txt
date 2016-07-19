
SET(CTCore_list
	console.cpp
	filexml.cpp
	maths.cpp
	pch.cpp
	pch.h
	SimpleMath.cpp
	utility.cpp
)

ADD_LIBRARY(
	CTCore SHARED
	${CTCore_list}
)
