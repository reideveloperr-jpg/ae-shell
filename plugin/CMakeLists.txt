include(${CMAKE_SOURCE_DIR}/cmake/PiPL.cmake)

add_library(ae-shell SHARED
    src/EntryPoint.cpp
    src/Commands.cpp
    src/AEBridge.cpp
    src/WindowHost.cpp
    src/Logging.cpp
    # AEGP_SuiteHandler.cpp ships in the AE SDK's Util/ folder and defines
    # the suite-caching class that EntryPoint.cpp / Commands.cpp use to
    # access AEGP_*Suite vtables. The header alone isn't enough -- without
    # the .cpp the linker errors with unresolved externals on every
    # AEGP_SuiteHandler member function.
    "${ADOBE_AE_SDK_SUITE_HANDLER_CPP}"
)

target_include_directories(ae-shell PRIVATE include)

target_link_libraries(ae-shell PRIVATE
    AdobeAESDK::AdobeAESDK
    ae-shell-ui
)

# After Effects loads any DLL with a `.aex` extension and a `PiPL` resource.
set_target_properties(ae-shell PROPERTIES
    PREFIX ""
    SUFFIX ".aex"
    OUTPUT_NAME "ae-shell"
)

# AE expects the AEGP entry point exported by name; explicit __declspec is in
# the source. No .def file needed.

ae_shell_add_pipl_resource(ae-shell
    PIPL_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/PiPL.r"
)

# Deploy Qt DLLs next to the .aex so AE can find them when it loads the plugin.
if(WIN32)
    find_program(WINDEPLOYQT_EXE windeployqt
        HINTS "${Qt6_DIR}/../../../bin"
              "$ENV{Qt6_DIR}/../../../bin"
        DOC "windeployqt utility for staging Qt DLLs")
    if(WINDEPLOYQT_EXE)
        add_custom_command(TARGET ae-shell POST_BUILD
            COMMAND "${WINDEPLOYQT_EXE}"
                    --no-translations
                    --no-compiler-runtime
                    --no-system-d3d-compiler
                    --no-opengl-sw
                    "$<TARGET_FILE:ae-shell>"
            COMMENT "Staging Qt DLLs alongside ae-shell.aex"
            VERBATIM)
    else()
        message(WARNING "windeployqt not found; you'll need to copy Qt DLLs next to ae-shell.aex by hand.")
    endif()
endif()

# AEGP plugins need to be x64 on modern AE.
if(MSVC)
    target_compile_definitions(ae-shell PRIVATE
        AE_OS_WIN
        MSWindows
    )
endif()
