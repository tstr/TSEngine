#####################################################################################################################
#
#	Resource Schema Generator
#
#####################################################################################################################

SET (rcschema_src
	src/main.cpp
	src/Schema.h
	src/Schema.cpp
	src/SchemaReader.h
	src/SchemaReader.cpp
	src/IGenerator.h
	src/generators/Cpp.h
	src/generators/Cpp.cpp
	
	inc/rcschema.h
)

ADD_EXECUTABLE(
	rcschema
	${rcschema_src}
)

assign_source_groups(${rcschema_src})

install_tools(rcschema)

TARGET_LINK_LIBRARIES(
	rcschema PRIVATE
	tscore
	CLIutil
)

#####################################################################################################################
#	Generated header dependencies
#####################################################################################################################

ADD_LIBRARY(rcschemaLib INTERFACE)
target_include_directories(rcschemaLib INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/inc/")
target_link_libraries(rcschema INTERFACE rcschemaLib)

#####################################################################################################################
#	Testing
#####################################################################################################################

if (TS_BUILD_TESTS)

# Test output directory
set(test_output_dir "${CMAKE_CURRENT_BINARY_DIR}/test/")
file(MAKE_DIRECTORY ${test_output_dir})

# Test inputs
set(rcschema_inputs
	test/Test0.schema
	test/Test1.schema
	test/Test2.schema
	test/Test3.schema
)

# Test outputs
set(rcschema_outputs
	"${test_output_dir}/Test0.rcs.h"
	"${test_output_dir}/Test1.rcs.h"
	"${test_output_dir}/Test2.rcs.h"
	"${test_output_dir}/Test3.rcs.h"
)

ADD_CUSTOM_COMMAND(
	OUTPUT ${rcschema_outputs}
	COMMAND "$<TARGET_FILE:rcschema>" --out ${test_output_dir} ${rcschema_inputs}
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	DEPENDS ${rcschema_inputs}
)

set_source_files_properties(${rcschema_outputs} PROPERTIES GENERATED TRUE)

set(test_rcschema_files
	test/test.cpp
	${rcschema_outputs}
	${rcschema_inputs}
)

# Add test runner
ADD_EXECUTABLE(
	TestRCSchema
	${test_rcschema_files}
)

assign_source_groups(${test_rcschema_files})

TARGET_INCLUDE_DIRECTORIES(TestRCSchema PRIVATE ${test_output_dir})

TARGET_LINK_LIBRARIES(
	TestRCSchema PRIVATE
	tscore
	rcschemaLib
)

# Add test suite
ADD_TEST(
	NAME TestRCSchema
	COMMAND "$<TARGET_FILE:TestRCSchema>"
)

SET_TARGET_PROPERTIES(
	TestRCSchema
	PROPERTIES FOLDER tools/tests
)

endif()

#####################################################################################################################
