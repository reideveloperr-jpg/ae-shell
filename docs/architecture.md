# AE Shell — Architecture

This document explains how the plugin and the UI fit together, what runs on
which thread, and how data flows between After Effects and our Qt window.

## Modules

| Module | Output | Depends on |
|---|---|---|
| `ae-shell-ui` (`ui/`) | static C++/Qt 6 library, plus `ae-shell-ui-standalone.exe` for development | Qt 6 (Core, Gui, Widgets) |
| `ae-shell` (`plugin/`) | `ae-shell.aex` (Windows DLL renamed) | `ae-shell-ui`, Adobe AE SDK |

The plugin **never** includes Qt headers directly. Communication with the UI
goes through the small `ae_shell::ui` API in [`ui/include/UiHost.h`](../ui/include/UiHost.h):

```cpp
namespace ae_shell::ui {
    struct EffectInfo { /* match_name, display_name, category, is_third_party */ };

    int  run_ui_event_loop();
    void raise_main_window();
    void request_quit();

    void set_effect_library(std::vector<EffectInfo> effects);
    std::vector<EffectInfo> current_effect_library();
}
```

This keeps a hard wall between AE-side code and Qt-side code: the AEGP layer
only sees POD types and a handful of free functions, while the UI module is
free to use any Qt facility. The shared `EffectInfo` struct is the single
"wire" type — AEBridge fills it on the AE thread, the UI panel reads it on
the Qt thread.

## Data flow: effect library

```
AE main thread                              Qt UI thread
──────────────                              ─────────────
CommandHook                                 (idle: app.exec)
  AEBridge::EnumerateInstalledEffects()
    └── AEGP_EffectSuite4::
        ├── AEGP_GetNumInstalledEffects
        └── AEGP_GetNextInstalledEffect ×N
            ├── AEGP_GetEffectMatchName
            ├── AEGP_GetEffectName
            └── AEGP_GetEffectCategory
  ▼
  std::vector<ui::EffectInfo>
  ▼
  ui::set_effect_library(std::move(effects))
    ├── std::lock_guard on g_effects_mutex
    └── QMetaObject::invokeMethod( ── queued signal ─►  MainWindow::refreshEffectLibrary()
            window, refreshEffectLibrary,                   └── EffectLibraryPanel::setEffects(snapshot)
            Qt::QueuedConnection)                                   └── rebuild + group + sort
  ▼
  WindowHost::Instance().Show()  ── first call only ─►  spawn worker, start app.exec()
```

The plugin re-enumerates on every menu invocation: the cost is a few hundred
short string copies, and it means newly-installed third-party plugins show
up the next time the user opens the panel without restarting AE.

## Process & threads

```
┌─ After Effects.exe ────────────────────────────────────────────────┐
│                                                                    │
│  AE main thread                                                    │
│   ├─ EntryPointFunc()  ◀─── called by AE during plugin scan         │
│   ├─ CommandHook()     ◀─── called when user picks "Window→AE Shell"│
│   │     └─ WindowHost::Instance().Show()                            │
│   │           ├─ first call: spawn worker thread, return            │
│   │           └─ subsequent calls: post raise_main_window() to Qt   │
│   ├─ UpdateMenuHook()  ◀─── enable/disable menu item                │
│   └─ AEGP_IdleHook (Phase 1+) drains Qt→AE command queue            │
│                                                                    │
│  UI worker thread (spawned by WindowHost on first Show)            │
│   ├─ run_ui_event_loop()                                           │
│   │     ├─ QApplication app(argc, argv)                             │
│   │     ├─ MainWindow window                                        │
│   │     ├─ window.show()                                            │
│   │     └─ app.exec()      ◀─── Qt event loop runs here             │
│   └─ on quit: returns; thread joins                                 │
└────────────────────────────────────────────────────────────────────┘
```

### Why a separate thread?

Two reasons:

1. **Don't block AE.** Qt's `app.exec()` is a blocking call that owns the
   thread it runs on. If we ran it on AE's main thread, AE would freeze while
   our window was open.
2. **Don't fight AE's event loop.** AE has its own message pump on the main
   thread. Qt installs its own event dispatcher; running both on the same
   thread invites deadlocks when one or the other has nested loops.

Putting Qt on a worker thread means each side keeps its own event dispatcher
and we explicitly hop between them when needed:

* `ui::raise_main_window()` posts a queued slot to the Qt thread.
* `AEBridge` (Phase 1+) calls run on the AE main thread; Qt code wanting to
  call AE pushes a closure into a thread-safe queue that an `AEGP_IdleHook`
  drains on the AE thread.

### Caveats

* On Windows, `QApplication` may live on a non-main thread, but it must own
  the message loop for that thread. Don't pass widget pointers across
  threads — always marshal via `QMetaObject::invokeMethod` with
  `Qt::QueuedConnection`.
* Some Adobe SDKs assume their callbacks land on the main thread. If we
  ever hit a "may only be called on main thread" assertion, the fix is to
  push the call to the AE-side queue instead of calling directly from Qt.

## Build pipeline for `ae-shell.aex`

```
PiPL.r ─┬─► (cl /EP)  ─► PiPL.rr
        └─► PiPLtool.exe ─► PiPL.rrc ─┐
                                       ▼
plugin/src/*.cpp ──► cl/link ─► ae-shell.dll ──► rename ──► ae-shell.aex
                       ▲
                       └─ links ae-shell-ui (static) + Qt6Widgets.dll, etc.
```

The PiPL resource is the magic that makes AE recognize the file. CMake glue
for it lives in [`cmake/PiPL.cmake`](../cmake/PiPL.cmake).

## Phase plan

See the [README roadmap](../README.md#roadmap) for the full table. Short version:

* **Phase 0** ✅ — scaffold, plugin loads, UI window opens.
* **Phase 1** ✅ — list every installed effect (incl. third-party) in the panel.
* **Phase 2** — apply effects to the active layer with one click.
* **Phase 3+** — custom timeline, drag-and-drop, parameter inspector,
  templates, render queue integration, polish.
