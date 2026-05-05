#pragma once

#include <string>
#include <vector>

namespace ae_shell {

// Plain-data view of an installed effect, suitable to ship across the
// AEGP/Qt thread boundary without dragging AE headers into the UI module.
struct EffectInfo {
    std::string match_name;     // unique internal id (e.g. "ADBE Gaussian Blur 2")
    std::string display_name;   // user-facing name (e.g. "Gaussian Blur")
    std::string category;       // AE category (e.g. "Blur & Sharpen")
    bool        is_third_party = false;  // heuristic: not "ADBE ..." prefix
};

// Façade over the AE SDK's various AEGP suites. Phase 0 only stubs the
// methods; Phase 1 fills in `EnumerateInstalledEffects`.
//
// Must be called on AE's main thread.
class AEBridge {
public:
    // Returns the list of every effect currently registered with AE,
    // including third-party / paid plugins like Sapphire, Trapcode, etc.
    [[nodiscard]] std::vector<EffectInfo> EnumerateInstalledEffects();

    // Phase 2 will add ApplyEffectToActiveLayer, etc.
};

}  // namespace ae_shell
