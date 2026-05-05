# preprocess_pipl.cmake
#
# Helper script invoked via `cmake -P` to run cl.exe in pre-processing mode
# and capture stdout into a file. We do this through a CMake script (rather
# than directly inside add_custom_command) because:
#
#   * `add_custom_command(... VERBATIM)` does NOT pass the command through a
#     shell, so the `>` redirect operator becomes a literal argv to cl.exe
#     and the .rr file never gets written.
#   * cl's `/Fi` flag is silently ignored when combined with `/EP` per the
#     MSVC docs ("/EP overrides /P"), so we cannot ask cl to write the file
#     directly while also stripping #line directives (which PiPLtool needs).
#
# Solution: drive cl.exe via execute_process(... OUTPUT_FILE ...), which
# is the cross-platform CMake way of capturing stdout to a file. No shell,
# no redirect operator, no quoting puzzles.
#
# Required defines (passed via -D on the cmake -P line):
#   CL_PATH    Absolute path to cl.exe.
#   SOURCE     Absolute path to the input .r file.
#   OUTPUT     Absolute path where the pre-processed .rr should be written.
#   INCLUDES   Semicolon-separated list of include directories.
#   DEFINES    Semicolon-separated list of preprocessor defines (optional).

if(NOT CL_PATH)
    message(FATAL_ERROR "preprocess_pipl.cmake: CL_PATH not set")
endif()
if(NOT SOURCE)
    message(FATAL_ERROR "preprocess_pipl.cmake: SOURCE not set")
endif()
if(NOT OUTPUT)
    message(FATAL_ERROR "preprocess_pipl.cmake: OUTPUT not set")
endif()

set(_cmd "${CL_PATH}" /nologo /EP /Tc "${SOURCE}")

if(DEFINES)
    foreach(_def IN LISTS DEFINES)
        if(_def)
            list(APPEND _cmd "/D" "${_def}")
        endif()
    endforeach()
endif()

if(INCLUDES)
    foreach(_inc IN LISTS INCLUDES)
        if(_inc)
            list(APPEND _cmd "-I${_inc}")
        endif()
    endforeach()
endif()

# Make sure the destination directory exists.
get_filename_component(_out_dir "${OUTPUT}" DIRECTORY)
file(MAKE_DIRECTORY "${_out_dir}")

execute_process(
    COMMAND ${_cmd}
    OUTPUT_FILE "${OUTPUT}"
    RESULT_VARIABLE _rc
    ERROR_VARIABLE  _stderr)

if(NOT _rc EQUAL 0)
    message(FATAL_ERROR
        "cl pre-processing failed with exit code ${_rc}\n"
        "Command: ${_cmd}\n"
        "Stderr: ${_stderr}")
endif()
