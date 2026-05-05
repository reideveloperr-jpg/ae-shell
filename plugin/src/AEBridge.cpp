// AEBridge.cpp — façade over the AE SDK's AEGP suites.
//
// Phase 0 only stubs out the methods that the UI will call from Phase 1
// onwards. They return empty/sentinel values so the UI compiles and runs
// in standalone mode without an AE host.

#include "AEBridge.h"

#include "Logging.h"
#include "PluginGlobals.h"

#include <AE_GeneralPlug.h>
#include <AEGP_SuiteHandler.h>

namespace ae_shell {

std::vector<EffectInfo> AEBridge::EnumerateInstalledEffects() {
    // Real implementation lands in Phase 1. The plan:
    //
    //   1. Get the active project handle via ProjSuite6.
    //   2. Walk effect registry through EffectSuite4:
    //        AEGP_GetNumInstalledEffects(&numEffectsL);
    //        for (i = 0; i < numEffectsL; ++i) {
    //            AEGP_GetNextInstalledEffect(prev, &cur);
    //            AEGP_GetEffectMatchName(cur, match_name);
    //            AEGP_GetEffectName(cur, display_name);
    //            AEGP_GetEffectCategory(cur, category);
    //        }
    //   3. Mark `is_third_party` = !match_name.starts_with("ADBE ").
    //
    // This works for paid third-party plugins because they register the
    // same way — the host doesn't distinguish between Adobe and 3rd party.
    log::Info("AEBridge::EnumerateInstalledEffects — stub (Phase 0)");
    return {};
}

}  // namespace ae_shell
