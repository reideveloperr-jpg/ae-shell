// UiHost.cpp — owns the QApplication lifecycle, the main window pointer, and
// the in-memory effect library that the plugin pushes to us.
//
// `run_ui_event_loop()` is called from the plugin's worker thread. It must
// be called exactly once per process. Subsequent menu invocations go through
// `raise_main_window()`, which schedules the raise on the Qt main thread via
// `QMetaObject::invokeMethod`.

#include "UiHost.h"

#include "MainWindow.h"
#include "Theme.h"

#include <QApplication>
#include <QMetaObject>
#include <QPointer>
#include <QtGlobal>

#include <atomic>
#include <mutex>
#include <utility>
#include <vector>

namespace ae_shell::ui {

namespace {

std::mutex                  g_window_mutex;
QPointer<MainWindow>        g_window;
std::atomic<bool>           g_running{false};

std::mutex                  g_effects_mutex;
std::vector<EffectInfo>     g_effects;

// Phase 0 keeps argv minimal. We give Qt a single fake program name so it
// can populate QCoreApplication::arguments() without crashing.
char  g_argv0[]   = "ae-shell";
char* g_argv[]    = { g_argv0, nullptr };
int   g_argc      = 1;

}  // namespace

int run_ui_event_loop() {
    bool expected = false;
    if (!g_running.compare_exchange_strong(expected, true)) {
        // Already running: defensive guard. Should not happen because the
        // plugin's WindowHost gates this with its own atomic.
        return -1;
    }

    // High-DPI is the default on Qt 6 — no Qt::AA_EnableHighDpiScaling
    // dance required.
    QApplication app(g_argc, g_argv);
    app.setOrganizationName("ae-shell");
    app.setApplicationName ("AE Shell");

    applyTheme(&app);

    MainWindow window;
    {
        std::lock_guard lk(g_window_mutex);
        g_window = &window;
    }
    window.show();

    const int rc = app.exec();

    {
        std::lock_guard lk(g_window_mutex);
        g_window = nullptr;
    }
    g_running.store(false);
    return rc;
}

void raise_main_window() {
    QPointer<MainWindow> w;
    {
        std::lock_guard lk(g_window_mutex);
        w = g_window;
    }
    if (!w) return;

    // Hop onto the Qt main thread before touching QWidget APIs.
    QMetaObject::invokeMethod(w.data(), [w]() {
        if (!w) return;
        w->showNormal();
        w->raise();
        w->activateWindow();
    }, Qt::QueuedConnection);
}

void request_quit() {
    if (auto* app = QCoreApplication::instance()) {
        QMetaObject::invokeMethod(app, []() {
            QCoreApplication::quit();
        }, Qt::QueuedConnection);
    }
}

void set_effect_library(std::vector<EffectInfo> effects) {
    {
        std::lock_guard lk(g_effects_mutex);
        g_effects = std::move(effects);
    }

    QPointer<MainWindow> w;
    {
        std::lock_guard lk(g_window_mutex);
        w = g_window;
    }
    if (!w) return;

    QMetaObject::invokeMethod(w.data(), [w]() {
        if (!w) return;
        w->refreshEffectLibrary();
    }, Qt::QueuedConnection);
}

std::vector<EffectInfo> current_effect_library() {
    std::lock_guard lk(g_effects_mutex);
    return g_effects;
}

}  // namespace ae_shell::ui
