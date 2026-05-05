#pragma once

#include "UiHost.h"  // canonical EffectInfo lives here, no Qt headers leak

#include <vector>

namespace ae_shell {

// Façade over the AE SDK's various AEGP suites.
//
// All methods must be called on AE's main thread; AE is not thread-safe and
// most suites assume the caller is the thread that received the suite
// pointer in EntryPointFunc.
class AEBridge {
public:
    // Returns the list of every effect currently registered with AE,
    // including third-party / paid plugins like Sapphire, Trapcode, etc.
    // The host doesn't distinguish Adobe-shipped from third-party effects;
    // we use the "ADBE " match-name prefix as a heuristic for the badge.
    [[nodiscard]] std::vector<ui::EffectInfo> EnumerateInstalledEffects();

    // Phase 2 will add ApplyEffectToActiveLayer, etc.
};

}  // namespace ae_shell
