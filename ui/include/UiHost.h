#pragma once

// Public C++ API of the ae-shell-ui module — the surface that the AEGP plugin
// links against. We intentionally keep no Qt types in this header so the
// plugin module never sees Qt headers.

namespace ae_shell::ui {

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

}  // namespace ae_shell::ui
