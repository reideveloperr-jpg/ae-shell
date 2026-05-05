// Commands.cpp — handlers for the "AE Shell" menu command.

#include <AE_GeneralPlug.h>
#include <AEGP_SuiteHandler.h>

#include "Logging.h"
#include "PluginGlobals.h"
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
    // Always enabled. Phase 1 will gate this on whether a project is open.
    return suites.CommandSuite1()->AEGP_EnableCommand(Globals().show_window_cmd);
}

}  // namespace ae_shell
