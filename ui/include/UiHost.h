#pragma once

#include <string>
#include <vector>

// Public C++ API of the ae-shell-ui module — the surface that the AEGP plugin
// links against. We intentionally keep no Qt types in this header so the
// plugin module never sees Qt headers.

namespace ae_shell::ui {

// Plain-data view of an installed AE effect.  Shared between the plugin's
// AEBridge and the UI's effect library panel.  Keeping it POD-only means
// the plugin doesn't need to pull in any Qt headers to build it.
struct EffectInfo {
    std::string match_name;     // unique internal id (e.g. "ADBE Gaussian Blur 2")
    std::string display_name;   // user-facing name (e.g. "Gaussian Blur")
    std::string category;       // AE category (e.g. "Blur & Sharpen")
    bool        is_third_party = false;  // !match_name.starts_with("ADBE ")
};

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

// Owns the QApplication and runs the Qt event loop until the user closes the
// main window or `request_quit()` is called.  Returns the QApplication's
// exit code.
//
// Thread: must be called on a dedicated worker thread; the plugin's
// WindowHost spawns one for this purpose. May be called once per process.
int run_ui_event_loop();

// Brings the main window to the front of the user's window stack on
// subsequent menu invocations. Safe to call from any thread.
void raise_main_window();

// Asks the running event loop to quit. Safe to call from any thread.
void request_quit();

// ---------------------------------------------------------------------------
// Data feed
// ---------------------------------------------------------------------------

// Replaces the in-memory effect library that the UI shows in its left-hand
// panel. Safe to call from any thread; if the main window is already up,
// the panel is refreshed via a queued slot on the Qt thread.
//
// The plugin calls this from AE's main thread immediately before
// WindowHost::Show() so the panel has data the moment the window appears.
void set_effect_library(std::vector<EffectInfo> effects);

// Returns a snapshot of the most recently-set effect library. Used by
// MainWindow's constructor to populate the panel on first show.
[[nodiscard]] std::vector<EffectInfo> current_effect_library();

}  // namespace ae_shell::ui
