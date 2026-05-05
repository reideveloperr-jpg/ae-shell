# FindAdobeAESDK.cmake
#
# Locates the Adobe After Effects SDK on the local filesystem.
#
# The SDK is NDA-licensed by Adobe and MUST NOT be committed to a public
# repository. Each developer downloads it from
# https://developer.adobe.com/after-effects/ and points the build at it via
# one of:
#
#   * `-DADOBE_AE_SDK=<path>` on the CMake command line
#   * `ADOBE_AE_SDK` environment variable
#   * `AE_SDK_PATH` environment variable (legacy fallback)
#
# Provides target `AdobeAESDK::AdobeAESDK` (interface library; headers only).
#
# Provides variables:
#   ADOBE_AE_SDK_FOUND      - TRUE if the SDK was located.
#   ADOBE_AE_SDK_ROOT       - Absolute path to the SDK root.
#   ADOBE_AE_SDK_INCLUDE_DIR
#   ADOBE_AE_SDK_RESOURCES_DIR
#   ADOBE_AE_SDK_PIPLTOOL   - Path to PiPLtool.exe (Windows only).

if(NOT ADOBE_AE_SDK)
    if(DEFINED ENV{ADOBE_AE_SDK})
        set(ADOBE_AE_SDK "$ENV{ADOBE_AE_SDK}")
    elseif(DEFINED ENV{AE_SDK_PATH})
        set(ADOBE_AE_SDK "$ENV{AE_SDK_PATH}")
    endif()
endif()

if(NOT ADOBE_AE_SDK)
    set(ADOBE_AE_SDK_FOUND FALSE)
    if(AdobeAESDK_FIND_REQUIRED)
        message(FATAL_ERROR
            "Adobe After Effects SDK not found.\n"
            "Download it from https://developer.adobe.com/after-effects/ and "
            "set ADOBE_AE_SDK either as a CMake cache variable "
            "(-DADOBE_AE_SDK=C:/path/to/AfterEffectsSDK) or as an environment "
            "variable.")
    endif()
    return()
endif()

get_filename_component(ADOBE_AE_SDK_ROOT "${ADOBE_AE_SDK}" ABSOLUTE)

# The AE SDK folder layout has changed across versions:
#   * Pre-2024: <root>/Headers, <root>/Resources, <root>/SP
#   * AE 2025+ (Zstandard archive): <root>/Examples/Headers, <root>/Examples/Resources,
#                                   <root>/Examples/Headers/SP
# We search both layouts so the same env var works regardless.
find_path(ADOBE_AE_SDK_INCLUDE_DIR
    NAMES AE_GeneralPlug.h
    HINTS
        "${ADOBE_AE_SDK_ROOT}/Headers"
        "${ADOBE_AE_SDK_ROOT}/Examples/Headers"
        "${ADOBE_AE_SDK_ROOT}/AfterEffectsSDK/Headers"
        "${ADOBE_AE_SDK_ROOT}/AfterEffectsSDK/Examples/Headers"
        "${ADOBE_AE_SDK_ROOT}/SDK/Headers"
    NO_DEFAULT_PATH)

find_path(ADOBE_AE_SDK_SP_INCLUDE_DIR
    NAMES SPBasic.h
    HINTS
        "${ADOBE_AE_SDK_INCLUDE_DIR}/SP"
        "${ADOBE_AE_SDK_ROOT}/Headers/SP"
        "${ADOBE_AE_SDK_ROOT}/Examples/Headers/SP"
        "${ADOBE_AE_SDK_ROOT}/AfterEffectsSDK/Headers/SP"
        "${ADOBE_AE_SDK_ROOT}/AfterEffectsSDK/Examples/Headers/SP"
    NO_DEFAULT_PATH)

# AEGP_SuiteHandler.{h,cpp} live in the SDK's Util/ folder. We need both
# the directory on the include path AND the .cpp added to plugin sources
# (it defines the AEGP_SuiteHandler class methods that EntryPoint.cpp etc.
# call into).
find_path(ADOBE_AE_SDK_UTIL_DIR
    NAMES AEGP_SuiteHandler.h
    HINTS
        "${ADOBE_AE_SDK_ROOT}/Util"
        "${ADOBE_AE_SDK_ROOT}/Examples/Util"
        "${ADOBE_AE_SDK_ROOT}/AfterEffectsSDK/Util"
        "${ADOBE_AE_SDK_ROOT}/AfterEffectsSDK/Examples/Util"
        "${ADOBE_AE_SDK_INCLUDE_DIR}/Util"
        "${ADOBE_AE_SDK_INCLUDE_DIR}"
    NO_DEFAULT_PATH)

find_file(ADOBE_AE_SDK_SUITE_HANDLER_CPP
    NAMES AEGP_SuiteHandler.cpp
    HINTS
        "${ADOBE_AE_SDK_UTIL_DIR}"
        "${ADOBE_AE_SDK_ROOT}/Util"
        "${ADOBE_AE_SDK_ROOT}/Examples/Util"
    NO_DEFAULT_PATH)

# PiPLtool.exe is the most reliable anchor for the Resources/ folder across
# SDK versions. (In pre-2024 SDKs we could anchor on AE_PluginData.h, but
# Adobe moved that file into Headers/ starting with the AE 2025 release.)
find_path(ADOBE_AE_SDK_RESOURCES_DIR
    NAMES PiPLtool.exe PIPL.h AE_PluginData.h
    HINTS
        "${ADOBE_AE_SDK_ROOT}/Resources"
        "${ADOBE_AE_SDK_ROOT}/Examples/Resources"
        "${ADOBE_AE_SDK_ROOT}/AfterEffectsSDK/Resources"
        "${ADOBE_AE_SDK_ROOT}/AfterEffectsSDK/Examples/Resources"
    NO_DEFAULT_PATH)

if(WIN32)
    find_program(ADOBE_AE_SDK_PIPLTOOL
        NAMES PiPLtool.exe PiPLtool
        HINTS
            "${ADOBE_AE_SDK_RESOURCES_DIR}"
            "${ADOBE_AE_SDK_ROOT}/Resources"
            "${ADOBE_AE_SDK_ROOT}/Examples/Resources"
            "${ADOBE_AE_SDK_ROOT}/AfterEffectsSDK/Resources"
            "${ADOBE_AE_SDK_ROOT}/AfterEffectsSDK/Examples/Resources"
        NO_DEFAULT_PATH)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(AdobeAESDK
    REQUIRED_VARS
        ADOBE_AE_SDK_ROOT
        ADOBE_AE_SDK_INCLUDE_DIR
        ADOBE_AE_SDK_SP_INCLUDE_DIR
        ADOBE_AE_SDK_RESOURCES_DIR
        ADOBE_AE_SDK_UTIL_DIR
        ADOBE_AE_SDK_SUITE_HANDLER_CPP)

if(WIN32 AND NOT ADOBE_AE_SDK_PIPLTOOL)
    message(WARNING
        "PiPLtool.exe not found under ${ADOBE_AE_SDK_RESOURCES_DIR}. "
        "The plugin .aex will not embed a PiPL resource and AE will refuse "
        "to load it. Make sure your AE SDK download is complete.")
endif()

# find_package_handle_standard_args(AdobeAESDK ...) sets AdobeAESDK_FOUND
# (matching the package name passed as the first argument). It does NOT set
# the legacy uppercase ADOBE_AE_SDK_FOUND on every CMake version (e.g. 3.20
# leaves it empty), so we must check the correctly-cased variable here or
# the imported target will never be created and downstream targets will
# fail with 'links to target "AdobeAESDK::AdobeAESDK" but the target was
# not found'.
if(AdobeAESDK_FOUND AND NOT TARGET AdobeAESDK::AdobeAESDK)
    add_library(AdobeAESDK::AdobeAESDK INTERFACE IMPORTED)
    target_include_directories(AdobeAESDK::AdobeAESDK INTERFACE
        "${ADOBE_AE_SDK_INCLUDE_DIR}"
        "${ADOBE_AE_SDK_SP_INCLUDE_DIR}"
        "${ADOBE_AE_SDK_RESOURCES_DIR}"
        "${ADOBE_AE_SDK_UTIL_DIR}")
endif()

# Mirror the legacy uppercase variant for callers that expected the older
# convention.
set(ADOBE_AE_SDK_FOUND ${AdobeAESDK_FOUND})

mark_as_advanced(
    ADOBE_AE_SDK_INCLUDE_DIR
    ADOBE_AE_SDK_SP_INCLUDE_DIR
    ADOBE_AE_SDK_RESOURCES_DIR
    ADOBE_AE_SDK_UTIL_DIR
    ADOBE_AE_SDK_SUITE_HANDLER_CPP
    ADOBE_AE_SDK_PIPLTOOL)
