cmake_minimum_required(VERSION 3.22)

project(ae-shell
    VERSION 0.1.0
    DESCRIPTION "CapCut-style C++/Qt 6 shell for Adobe After Effects"
    LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(MSVC)
    add_compile_options(/W4 /permissive- /Zc:__cplusplus /utf-8 /MP)
    add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS)
endif()

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(AE_SHELL_BUILD_PLUGIN "Build the AEGP plugin (.aex). Requires AE SDK." ON)
option(AE_SHELL_BUILD_UI     "Build the Qt 6 UI module."                       ON)
option(AE_SHELL_BUILD_STANDALONE_UI
       "Build the Qt UI as a standalone .exe for development."                 ON)

# ---------------------------------------------------------------------------
# Qt
# ---------------------------------------------------------------------------
if(AE_SHELL_BUILD_UI)
    find_package(Qt6 6.2 REQUIRED COMPONENTS Core Gui Widgets)
    if(COMMAND qt_standard_project_setup)
        qt_standard_project_setup()
    endif()
endif()

# ---------------------------------------------------------------------------
# Adobe After Effects SDK
# ---------------------------------------------------------------------------
if(AE_SHELL_BUILD_PLUGIN)
    if(NOT WIN32)
        message(FATAL_ERROR
            "AE_SHELL_BUILD_PLUGIN=ON but the host platform is not Windows. "
            "Re-run with -DAE_SHELL_BUILD_PLUGIN=OFF, or build on Windows.")
    endif()
    find_package(AdobeAESDK REQUIRED)
endif()

# ---------------------------------------------------------------------------
# Subprojects
# ---------------------------------------------------------------------------
if(AE_SHELL_BUILD_UI)
    add_subdirectory(ui)
endif()

if(AE_SHELL_BUILD_PLUGIN)
    add_subdirectory(plugin)
endif()
