#
#   Get info about the latest git commit
#

# Find GIT_EXECUTABLE path
find_package(Git QUIET)

function (get_git_commit_desc _date_var _subject_var)
    #the date of the commit
    execute_process(COMMAND
      "${GIT_EXECUTABLE}" log -1 --format=%ad --date=local
      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
      OUTPUT_VARIABLE ${_date_var}
      ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

    #the subject of the commit
    execute_process(COMMAND
      "${GIT_EXECUTABLE}" log -1 --format=%s
      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
      OUTPUT_VARIABLE ${_subject_var}
      ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

    set(${_date_var} ${${_date_var}} PARENT_SCOPE)
    set(${_subject_var} ${${_subject_var}} PARENT_SCOPE)
endfunction()