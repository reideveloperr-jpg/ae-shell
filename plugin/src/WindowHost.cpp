// WindowHost.cpp — bridges the AE main thread (where the menu command fires)
// to the Qt event loop (where our UI lives).
//
// Architecture:
//   * AE calls our CommandHook on its main thread.
//   * We spin up a worker `std::thread` the first time, which calls into the
//     UI module's `ae_shell::ui::run_ui_event_loop()`.
//   * Inside that function, the UI module owns the QApplication.
//   * Subsequent menu invocations forward a "raise window" signal via the
//     UI module's `ae_shell::ui::raise_main_window()`.
//
// We deliberately keep all Qt headers out of the plugin module so the AEGP
// side has zero Qt link surface; only the `ae-shell-ui` static library does.

#include "WindowHost.h"

#include "Logging.h"

#include <atomic>

// Public C++ API exported by the ae-shell-ui module.
namespace ae_shell::ui {
int  run_ui_event_loop();
void raise_main_window();
void request_quit();
}  // namespace ae_shell::ui

namespace ae_shell {

WindowHost& WindowHost::Instance() {
    static WindowHost host;
    return host;
}

void WindowHost::Show() {
    bool expected = false;
    if (started_.compare_exchange_strong(expected, true)) {
        log::Info("WindowHost: spawning UI thread");
        ui_thread_ = std::thread([this]() { RunUiThread(); });
    } else {
        log::Info("WindowHost: raising existing UI window");
        ae_shell::ui::raise_main_window();
    }
}

void WindowHost::Shutdown() {
    if (started_.load()) {
        log::Info("WindowHost: requesting UI quit");
        ae_shell::ui::request_quit();
        if (ui_thread_.joinable()) {
            ui_thread_.join();
        }
        started_.store(false);
    }
}

WindowHost::~WindowHost() {
    Shutdown();
}

void WindowHost::RunUiThread() {
    log::Info("WindowHost: UI thread entering Qt event loop");
    const int rc = ae_shell::ui::run_ui_event_loop();
    log::Info(rc == 0
        ? "WindowHost: UI thread exited cleanly"
        : "WindowHost: UI thread exited with non-zero code");
}

}  // namespace ae_shell
