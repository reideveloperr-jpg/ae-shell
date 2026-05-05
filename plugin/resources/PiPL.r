// PiPL.r — Plug-in Property List for the AE Shell AEGP plugin.
//
// After Effects scans every `.aex` it finds in its Plug-ins folder for an
// embedded `PiPL` resource describing the plugin's kind, name, version, and
// the entry-point symbol name to call. Without this resource AE silently
// skips the file.
//
// The `.r` format is Mac-resource-style; on Windows we pre-process it with
// the C compiler and then run Adobe's `PiPLtool.exe` (shipped in the AE
// SDK's Resources folder) to convert it into a .rrc file that the MSVC
// resource compiler can embed in the DLL.

#include "AEConfig.h"
#include "AE_EffectVers.h"

#ifndef AE_OS_WIN
#   include <AE_General.r>
#endif

resource 'PiPL' (16000) {
    {
        Kind        { AEGP },
        Name        { "AE Shell" },
        Category    { "AE Shell" },
#ifdef AE_OS_WIN
    #ifdef AE_PROC_INTELx64
        CodeWin64X86 { "EntryPointFunc" },
    #endif
#else
    #ifdef AE_OS_MAC
        #ifdef AE_ARM64
            CodeMacARM64 { "EntryPointFunc" },
        #endif
        #ifdef AE_PROC_INTELx64
            CodeMacIntel64 { "EntryPointFunc" },
        #endif
    #endif
#endif
// AE_PluginVersion is declared in <AE_General.r>, which is only #include'd
// on Mac (it's a Rez-only header that the Windows toolchain pre-processes
// PiPL.r without). Emitting AE_PluginVersion on Windows therefore causes
// PiPLtool to fail with "ParsePiPLBody: Unexpected Token". Restrict it to
// non-Windows targets; AE does not require this property to load the
// plugin and the VS_VERSION_INFO resource on Windows already carries the
// plugin's version.
#ifndef AE_OS_WIN
        AE_PluginVersion { PF_PLUG_IN_VERSION, PF_PLUG_IN_SUBVERS },
#endif
    }
};
