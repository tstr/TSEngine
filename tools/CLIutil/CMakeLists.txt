#
#	CLIutil:
#		Provides utility classes/functions used by CLI applications
#

#######################################################################################################################

SET (cli_inc
	inc/cli/Arguments.h
	inc/cli/Constants.h
)

SET (cli_src
	src/Arguments.cpp
)

ADD_LIBRARY(
	CLIutil
	STATIC
	${cli_inc} ${cli_src}
)

install_tools(CLIutil)

assign_source_groups(${cli_inc} ${cli_src})

TARGET_INCLUDE_DIRECTORIES(
	CLIutil PUBLIC
	inc/
)

TARGET_LINK_LIBRARIES(
	CLIutil PRIVATE
	tscore
)
#####################################################################################################################
