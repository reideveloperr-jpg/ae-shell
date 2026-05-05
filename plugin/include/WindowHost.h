#pragma once

#include <atomic>
#include <thread>

namespace ae_shell {

// Owns the worker thread that runs Qt's QApplication and the main window.
// The first time the user invokes the "AE Shell" menu command we spin up the
// thread; subsequent invocations bring the existing window forward.
class WindowHost {
public:
    static WindowHost& Instance();

    // Idempotent: starts the Qt thread on first call, raises the existing
    // window on subsequent calls. Safe to call from AE's main thread.
    void Show();

    // Called from EntryPointFunc cleanup (rare — AE seldom unloads plugins).
    void Shutdown();

private:
    WindowHost() = default;
    ~WindowHost();
    WindowHost(const WindowHost&)            = delete;
    WindowHost& operator=(const WindowHost&) = delete;

    void RunUiThread();

    std::thread       ui_thread_;
    std::atomic<bool> started_{false};
};

}  // namespace ae_shell
