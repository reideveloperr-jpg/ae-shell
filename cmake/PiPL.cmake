# PiPL.cmake
#
# Helper to generate a PiPL resource for an AEGP plugin.
#
# An AE plugin is a normal Windows DLL renamed `.aex`, with a single special
# resource called `PiPL` (Plug-in Property List) that AE reads during plugin
# discovery. Without it, AE will not load the plugin.
#
# The resource is described in a Mac-style `.r` file. Adobe ships
# `PiPLtool.exe` with the AE SDK to compile that into a `.rsrc` file, which
# we then reference from a Windows `.rc` file via `RCDATA`.
#
# Usage:
#   ae_shell_add_pipl_resource(<target>
#       PIPL_FILE  <path/to/PiPL.r>
#       PIPL_INCLUDES <extra include dirs for PiPLtool>)

function(ae_shell_add_pipl_resource TARGET)
    cmake_parse_arguments(ARG "" "PIPL_FILE" "PIPL_INCLUDES" ${ARGN})

    if(NOT ARG_PIPL_FILE)
        message(FATAL_ERROR "ae_shell_add_pipl_resource requires PIPL_FILE")
    endif()
    if(NOT WIN32)
        message(FATAL_ERROR "PiPL build is only supported on Windows for now")
    endif()
    if(NOT ADOBE_AE_SDK_PIPLTOOL)
        message(FATAL_ERROR
            "PiPLtool.exe not found. Cannot build PiPL resource for ${TARGET}. "
            "Re-run CMake with a valid ADOBE_AE_SDK pointing to a complete "
            "AE SDK download.")
    endif()

    get_filename_component(PIPL_NAME ${ARG_PIPL_FILE} NAME_WE)
    set(GEN_DIR  "${CMAKE_CURRENT_BINARY_DIR}/pipl")
    set(RR_FILE  "${GEN_DIR}/${PIPL_NAME}.rr")
    set(RC_FILE  "${GEN_DIR}/${PIPL_NAME}.rrc")
    file(MAKE_DIRECTORY "${GEN_DIR}")

    set(PIPL_INC_FLAGS "")
    foreach(_inc IN LISTS ARG_PIPL_INCLUDES ADOBE_AE_SDK_INCLUDE_DIR ADOBE_AE_SDK_RESOURCES_DIR)
        list(APPEND PIPL_INC_FLAGS "-I${_inc}")
    endforeach()

    # Stage 1: pre-process the .r file with cl.exe so PiPLtool sees a single
    # flat file with all `#include`s expanded.
    #
    # We use `/P /EP /Fi:<path>` instead of `/EP > file` because in a CMake
    # add_custom_command(VERBATIM) the `>` is *not* interpreted as a shell
    # redirect — it gets passed through as a literal argv to cl.exe, which
    # then errors with "The syntax of the command is incorrect" and never
    # writes the .rr file. `/Fi:<path>` makes cl write the preprocessed
    # output directly to disk, no shell needed.
    add_custom_command(
        OUTPUT  "${RR_FILE}"
        COMMAND "${CMAKE_C_COMPILER}"
                /nologo /P /EP /D "MSWindows" /Tc "${ARG_PIPL_FILE}"
                "/Fi${RR_FILE}"
                ${PIPL_INC_FLAGS}
        DEPENDS "${ARG_PIPL_FILE}"
        COMMENT "Pre-processing PiPL ${PIPL_NAME}.r -> ${PIPL_NAME}.rr"
        VERBATIM)

    # Stage 2: PiPLtool turns the pre-processed .rr into a Windows .rrc that
    # contains an `RCDATA "PiPL"` entry.
    add_custom_command(
        OUTPUT  "${RC_FILE}"
        COMMAND "${ADOBE_AE_SDK_PIPLTOOL}" "${RR_FILE}" "${RC_FILE}"
        DEPENDS "${RR_FILE}"
        COMMENT "Compiling PiPL with PiPLtool"
        VERBATIM)

    # Stage 3: feed the .rrc to the target's resource compiler.
    target_sources(${TARGET} PRIVATE "${RC_FILE}")
    set_source_files_properties("${RC_FILE}" PROPERTIES LANGUAGE RC)
endfunction()
