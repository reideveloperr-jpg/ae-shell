// EntryPoint.cpp — AEGP plugin entry point.
//
// After Effects looks up the symbol name declared in our PiPL resource
// (`EntryPointFunc`) and calls it once at AE startup. We use that callback to
// register the "Window → AE Shell" menu command and stash a few globals.

#include <AE_GeneralPlug.h>
#include <AEGP_SuiteHandler.h>

#if defined(_WIN32)
#   include <windows.h>
#endif

#include <cstdio>

#include "AEBridge.h"
#include "Logging.h"
#include "PluginGlobals.h"
#include "WindowHost.h"

#if defined(_MSC_VER)
#   define AE_SHELL_EXPORT extern "C" __declspec(dllexport)
#else
#   define AE_SHELL_EXPORT extern "C"
#endif

namespace ae_shell {

PluginGlobals& Globals() {
    static PluginGlobals g;
    return g;
}

// Forward decls for hooks defined in Commands.cpp.
A_Err CommandHook(
    AEGP_GlobalRefcon   plugin_refconPV,
    AEGP_CommandRefcon  refconPV,
    AEGP_Command        command,
    AEGP_HookPriority   hook_priority,
    A_Boolean           already_handledB,
    A_Boolean*          handledPB);

A_Err UpdateMenuHook(
    AEGP_GlobalRefcon       plugin_refconPV,
    AEGP_UpdateMenuRefcon   refconPV,
    AEGP_WindowType         active_window);

}  // namespace ae_shell

namespace {

// Helper to convert an A_Err to a printable hex string. AE's A_Err is just
// an int with implementation-defined codes; the actual values are documented
// in AE_GeneralPlug.h but for diagnostic logging we just want the raw value.
std::string ErrToHex(A_Err err) {
    char buf[32] = {};
    std::snprintf(buf, sizeof(buf), "0x%08lx", static_cast<unsigned long>(err));
    return buf;
}

A_Err EntryPointBody(
    SPBasicSuite*       pica_basicP,
    AEGP_PluginID       aegp_plugin_id)
{
    using namespace ae_shell;

    log::Info("Entered EntryPointBody, instantiating SuiteHandler");

    Globals().basic_suite = pica_basicP;
    Globals().plugin_id   = aegp_plugin_id;

    AEGP_SuiteHandler suites(pica_basicP);

    A_Err err = A_Err_NONE;

    log::Info("Calling AEGP_GetUniqueCommand");
    AEGP_Command cmd = 0;
    err = suites.CommandSuite1()->AEGP_GetUniqueCommand(&cmd);
    if (err) {
        log::Error("AEGP_GetUniqueCommand failed: " + ErrToHex(err));
        return err;
    }
    log::Info("AEGP_GetUniqueCommand ok, cmd=" + std::to_string(cmd));

    // The "\t" splits the menu label into (menu text, shortcut hint). AE
    // shows the first part as the visible menu item and the second part on
    // the right side; we leave the shortcut unset for now.
    // AE 2025 SDK renamed AEGP_InstallCommand -> AEGP_InsertMenuCommand and
    // dropped the AEGP_MenuID_* prefix in favor of AEGP_Menu_*. The type
    // AEGP_MenuID is still the parameter type (it's now a typedef of
    // A_LegacyEnumType). AEGP_MENU_INSERT_AT_BOTTOM is unchanged.
    log::Info("Calling AEGP_InsertMenuCommand");
    err = suites.CommandSuite1()->AEGP_InsertMenuCommand(
        cmd,
        "AE Shell\tAE Shell",
        AEGP_Menu_WINDOW,
        AEGP_MENU_INSERT_AT_BOTTOM);
    if (err) {
        log::Error("AEGP_InsertMenuCommand failed: " + ErrToHex(err));
        return err;
    }
    log::Info("AEGP_InsertMenuCommand ok");

    log::Info("Calling AEGP_RegisterCommandHook");
    err = suites.RegisterSuite5()->AEGP_RegisterCommandHook(
        aegp_plugin_id,
        AEGP_HP_BeforeAE,
        cmd,
        &CommandHook,
        nullptr);
    if (err) {
        log::Error("AEGP_RegisterCommandHook failed: " + ErrToHex(err));
        return err;
    }

    log::Info("Calling AEGP_RegisterUpdateMenuHook");
    err = suites.RegisterSuite5()->AEGP_RegisterUpdateMenuHook(
        aegp_plugin_id,
        &UpdateMenuHook,
        nullptr);
    if (err) {
        log::Error("AEGP_RegisterUpdateMenuHook failed: " + ErrToHex(err));
        return err;
    }

    Globals().show_window_cmd = cmd;
    log::Info("AE Shell: menu command registered successfully");
    return A_Err_NONE;
}

}  // namespace

AE_SHELL_EXPORT A_Err
EntryPointFunc(
    SPBasicSuite*       pica_basicP,
    A_long              major_versionL,
    A_long              minor_versionL,
    AEGP_PluginID       aegp_plugin_id,
    AEGP_GlobalRefcon*  /*global_refconP*/)
{
    using namespace ae_shell;

    // Push to the debugger stream BEFORE we touch logging or anything else
    // -- if our DLL crashes during a static initializer or AE terminates the
    // process before our log file flushes, this is the only trace left.
#if defined(_WIN32)
    OutputDebugStringA("[ae-shell] EntryPointFunc: native entry point invoked\n");
#endif

    log::Init();
    log::Info("AE Shell: EntryPointFunc invoked, host AE version " +
              std::to_string(major_versionL) + "." +
              std::to_string(minor_versionL));

    A_Err err = A_Err_NONE;

#if defined(_WIN32)
    // Trap any structured exception (access violation, illegal instruction,
    // etc.) so we always leave a final breadcrumb in the log even if AE then
    // tears down the plugin process.
    __try {
        err = EntryPointBody(pica_basicP, aegp_plugin_id);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        DWORD code = GetExceptionCode();
        char  buf[64] = {};
        std::snprintf(buf, sizeof(buf),
            "SEH exception in EntryPointBody, code=0x%08lx",
            static_cast<unsigned long>(code));
        log::Error(buf);
        return A_Err_GENERIC;
    }
#else
    err = EntryPointBody(pica_basicP, aegp_plugin_id);
#endif

    if (err) {
        log::Error("EntryPointFunc returning err " + ErrToHex(err));
    } else {
        log::Info("EntryPointFunc returning A_Err_NONE");
    }
    return err;
}
