// Commands.cpp — handlers for the "AE Shell" menu command.

#include <AE_GeneralPlug.h>
#include <AEGP_SuiteHandler.h>

#include "AEBridge.h"
#include "Logging.h"
#include "PluginGlobals.h"
#include "UiHost.h"
#include "WindowHost.h"

namespace ae_shell {

A_Err CommandHook(
    AEGP_GlobalRefcon   /*plugin_refconPV*/,
    AEGP_CommandRefcon  /*refconPV*/,
    AEGP_Command        command,
    AEGP_HookPriority   /*hook_priority*/,
    A_Boolean           /*already_handledB*/,
    A_Boolean*          handledPB)
{
    if (handledPB) *handledPB = FALSE;

    if (command != Globals().show_window_cmd) {
        return A_Err_NONE;
    }

    log::Info("AE Shell: command invoked, opening UI");

    // Refresh the effect library on every menu invocation. Enumeration is
    // cheap (a few hundred string copies) and this way newly-installed
    // plugins show up the next time the user opens the panel without
    // requiring an AE restart.
    AEBridge bridge;
    auto effects = bridge.EnumerateInstalledEffects();
    ui::set_effect_library(std::move(effects));

    WindowHost::Instance().Show();

    if (handledPB) *handledPB = TRUE;
    return A_Err_NONE;
}

A_Err UpdateMenuHook(
    AEGP_GlobalRefcon       /*plugin_refconPV*/,
    AEGP_UpdateMenuRefcon   /*refconPV*/,
    AEGP_WindowType         /*active_window*/)
{
    AEGP_SuiteHandler suites(Globals().basic_suite);
    // Always enabled. Phase 2 will gate this on whether a project is open.
    return suites.CommandSuite1()->AEGP_EnableCommand(Globals().show_window_cmd);
}

}  // namespace ae_shell
