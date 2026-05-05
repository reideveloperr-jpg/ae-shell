qt_add_library(ae-shell-ui STATIC
    src/UiHost.cpp
    src/MainWindow.cpp
    src/EffectLibraryPanel.cpp
    src/TimelinePanel.cpp
    src/InspectorPanel.cpp
    src/Theme.cpp
    include/UiHost.h
    include/MainWindow.h
    include/EffectLibraryPanel.h
    include/TimelinePanel.h
    include/InspectorPanel.h
    include/Theme.h
)

target_include_directories(ae-shell-ui PUBLIC include)
target_link_libraries(ae-shell-ui PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets)

# Position-independent code so this static lib can be linked into a DLL/.aex.
set_target_properties(ae-shell-ui PROPERTIES POSITION_INDEPENDENT_CODE ON)

qt_add_resources(ae-shell-ui ae_shell_ui_resources
    PREFIX "/ui"
    FILES
        resources/style.qss
)

if(AE_SHELL_BUILD_STANDALONE_UI)
    qt_add_executable(ae-shell-ui-standalone
        src/standalone_main.cpp
    )
    target_link_libraries(ae-shell-ui-standalone PRIVATE ae-shell-ui)
    set_target_properties(ae-shell-ui-standalone PROPERTIES
        WIN32_EXECUTABLE ON
        MACOSX_BUNDLE    ON
    )

    # Stage Qt DLLs alongside the standalone .exe on Windows.
    if(WIN32)
        find_program(WINDEPLOYQT_EXE windeployqt
            HINTS "${Qt6_DIR}/../../../bin"
                  "$ENV{Qt6_DIR}/../../../bin")
        if(WINDEPLOYQT_EXE)
            add_custom_command(TARGET ae-shell-ui-standalone POST_BUILD
                COMMAND "${WINDEPLOYQT_EXE}"
                        --no-translations
                        --no-compiler-runtime
                        --no-system-d3d-compiler
                        --no-opengl-sw
                        "$<TARGET_FILE:ae-shell-ui-standalone>"
                COMMENT "Staging Qt DLLs for ae-shell-ui-standalone"
                VERBATIM)
        endif()
    endif()
endif()
