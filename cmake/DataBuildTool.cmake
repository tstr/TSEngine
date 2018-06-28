#
#   Data Builder tool
#

include(PythonUtils)

set(DATABUILD_INPUT_DIR  "${PROJECT_SOURCE_DIR}/data")
set(DATABUILD_OUTPUT_DIR "${PROJECT_BINARY_DIR}/data")
set(DATABUILD_TOOL "${PYWRAPPER_COMMAND}" "${PROJECT_SOURCE_DIR}/tools/dbuild.py")

file(MAKE_DIRECTORY ${DATABUILD_OUTPUT_DIR})

install(
    DIRECTORY "${DATABUILD_OUTPUT_DIR}/"
    DESTINATION "data"
    PATTERN ".ninja_deps" EXCLUDE
    PATTERN ".ninja_log" EXCLUDE
    PATTERN "build.ninja" EXCLUDE
    PATTERN "index.dat" EXCLUDE
)

add_custom_target(DATA_BUILD ALL
	WORKING_DIRECTORY ${DATABUILD_OUTPUT_DIR}
    COMMAND ${DATABUILD_TOOL} "--build" "${DATABUILD_INPUT_DIR}"
)

add_custom_target(DATA_CLEAN ALL
    COMMAND ${CMAKE_COMMAND} -E echo "cleaning output directory: \"${DATABUILD_OUTPUT_DIR}\""
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${DATABUILD_OUTPUT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DATABUILD_OUTPUT_DIR}"
)

set_target_properties(
	DATA_BUILD DATA_CLEAN
	PROPERTIES FOLDER commands
)

set_target_properties(DATA_CLEAN PROPERTIES EXCLUDE_FROM_ALL TRUE)