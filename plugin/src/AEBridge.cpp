// AEBridge.cpp — façade over the AE SDK's AEGP suites.
//
// Phase 1: real implementation of EnumerateInstalledEffects() against
// AEGP_EffectSuite4. The list includes every effect AE has registered, so
// third-party paid plugins (Trapcode, Sapphire, Element 3D, …) appear in
// it the same way Adobe-shipped effects do.

#include "AEBridge.h"

#include "Logging.h"
#include "PluginGlobals.h"

#include <AE_GeneralPlug.h>
#include <AEGP_SuiteHandler.h>

#include <cstring>
#include <string>
#include <string_view>

namespace ae_shell {

namespace {

constexpr std::string_view kAdobePrefix = "ADBE ";

bool starts_with(std::string_view haystack, std::string_view needle) {
    return haystack.size() >= needle.size()
        && std::memcmp(haystack.data(), needle.data(), needle.size()) == 0;
}

}  // namespace

std::vector<ui::EffectInfo> AEBridge::EnumerateInstalledEffects() {
    std::vector<ui::EffectInfo> result;

    if (!Globals().basic_suite) {
        log::Error("AEBridge::EnumerateInstalledEffects called before EntryPointFunc");
        return result;
    }

    AEGP_SuiteHandler suites(Globals().basic_suite);
    auto* fx = suites.EffectSuite4();
    if (!fx) {
        log::Error("EffectSuite4 not available");
        return result;
    }

    A_long num = 0;
    A_Err err = fx->AEGP_GetNumInstalledEffects(&num);
    if (err) {
        log::Error("AEGP_GetNumInstalledEffects failed");
        return result;
    }
    log::Info(std::string("AE reports ") + std::to_string(num) + " installed effects");

    result.reserve(static_cast<std::size_t>(num));

    AEGP_InstalledEffectKey key = AEGP_InstalledEffectKey_NONE;
    for (A_long i = 0; i < num; ++i) {
        err = fx->AEGP_GetNextInstalledEffect(key, &key);
        if (err) {
            log::Error("AEGP_GetNextInstalledEffect failed mid-walk; stopping");
            break;
        }

        // AEGP_MAX_*_SIZE constants are defined in AE_EffectCB.h /
        // AE_EffectSuites.h. We use generous local buffers to be safe across
        // SDK versions.
        A_char match_name[AEGP_MAX_EFFECT_MATCH_NAME_SIZE]    = {};
        A_char display_name[AEGP_MAX_EFFECT_NAME_SIZE]        = {};
        A_char category[AEGP_MAX_EFFECT_CATEGORY_NAME_SIZE]   = {};

        if (fx->AEGP_GetEffectMatchName(key, match_name)) {
            log::Error("AEGP_GetEffectMatchName failed; skipping entry");
            continue;
        }
        // GetEffectName/GetEffectCategory are non-fatal: an empty name is
        // still useful (the match_name fallback covers it in the UI).
        fx->AEGP_GetEffectName    (key, display_name);
        fx->AEGP_GetEffectCategory(key, category);

        ui::EffectInfo info;
        info.match_name     = match_name;
        info.display_name   = display_name[0] ? std::string(display_name) : info.match_name;
        info.category       = category;
        info.is_third_party = !starts_with(info.match_name, kAdobePrefix);

        result.push_back(std::move(info));
    }

    log::Info(std::string("Enumeration finished: ") + std::to_string(result.size()) + " entries");
    return result;
}

}  // namespace ae_shell
