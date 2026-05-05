#pragma once

#include <AE_GeneralPlug.h>

namespace ae_shell {

// One-per-plugin state initialised in EntryPointFunc and consulted by
// command/menu hooks. AEGP plugins are loaded once per AE session, so a
// process-wide singleton is the natural fit.
struct PluginGlobals {
    SPBasicSuite*    basic_suite     = nullptr;
    AEGP_PluginID    plugin_id       = 0;
    AEGP_Command     show_window_cmd = 0;
};

PluginGlobals& Globals();

}  // namespace ae_shell
