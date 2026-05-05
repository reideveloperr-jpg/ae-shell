// EntryPoint.cpp — AEGP plugin entry point.
//
// After Effects looks up the symbol name declared in our PiPL resource
// (`EntryPointFunc`) and calls it once at AE startup. We use that callback to
// register the "Window → AE Shell" menu command and stash a few globals.

#include <AE_GeneralPlug.h>
#include <AEGP_SuiteHandler.h>

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

AE_SHELL_EXPORT A_Err
EntryPointFunc(
    SPBasicSuite*       pica_basicP,
    A_long              /*major_versionL*/,
    A_long              /*minor_versionL*/,
    AEGP_PluginID       aegp_plugin_id,
    AEGP_GlobalRefcon*  /*global_refconP*/)
{
    using namespace ae_shell;

    log::Init();
    log::Info("AE Shell: EntryPointFunc invoked");

    Globals().basic_suite = pica_basicP;
    Globals().plugin_id   = aegp_plugin_id;

    AEGP_SuiteHandler suites(pica_basicP);

    A_Err err = A_Err_NONE;

    AEGP_Command cmd = 0;
    err = suites.CommandSuite1()->AEGP_GetUniqueCommand(&cmd);
    if (err) {
        log::Error("AEGP_GetUniqueCommand failed");
        return err;
    }

    // The "\t" splits the menu label into (menu text, shortcut hint). AE
    // shows the first part as the visible menu item and the second part on
    // the right side; we leave the shortcut unset for now.
    err = suites.CommandSuite1()->AEGP_InstallCommand(
        cmd,
        "AE Shell\tAE Shell",
        AEGP_MenuID_WINDOW,
        AEGP_MENU_INSERT_AT_BOTTOM);
    if (err) {
        log::Error("AEGP_InstallCommand failed");
        return err;
    }

    err = suites.RegisterSuite5()->AEGP_RegisterCommandHook(
        aegp_plugin_id,
        AEGP_HP_BeforeAE,
        cmd,
        &CommandHook,
        nullptr);
    if (err) {
        log::Error("AEGP_RegisterCommandHook failed");
        return err;
    }

    err = suites.RegisterSuite5()->AEGP_RegisterUpdateMenuHook(
        aegp_plugin_id,
        &UpdateMenuHook,
        nullptr);
    if (err) {
        log::Error("AEGP_RegisterUpdateMenuHook failed");
        return err;
    }

    Globals().show_window_cmd = cmd;
    log::Info("AE Shell: menu command registered");
    return A_Err_NONE;
}
