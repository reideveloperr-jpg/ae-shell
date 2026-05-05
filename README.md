# AE Shell

A CapCut-style C++/Qt 6 shell for **Adobe After Effects**, implemented as an
AEGP plugin. The goal: a simplified, modern UI for everyday motion-graphics
tasks that still leverages the full After Effects rendering engine — including
**all installed third-party effects** (Trapcode, Sapphire, BorisFX, Element
3D, Red Giant, etc.), since they are applied by AE itself.

> **Status: Phase 0 (initial scaffold)** — builds an empty plugin and a Qt
> window. The "CapCut-style" UI is stubbed; the AE bridge enumerates and
> applies effects starting in Phase 1. See [Roadmap](#roadmap).

---

## How it works

```
┌──────────────────── After Effects (host process) ────────────────────┐
│                                                                       │
│  ┌──── ae-shell.aex (AEGP plugin, C++) ────────┐                     │
│  │  EntryPoint  ─→  registers menu command     │                     │
│  │                  Window → AE Shell           │                     │
│  │  AEBridge    ─→  wraps AEGP suites           │                     │
│  │                  (Proj/Comp/Layer/Effect/    │                     │
│  │                   Keyframe/RenderQueue)      │                     │
│  │  WindowHost  ─→  spawns Qt UI on worker     │                     │
│  │                  thread; idle hook drains    │                     │
│  │                  Qt → AE command queue       │                     │
│  └──────────────┬───────────────────────────────┘                     │
│                 │ in-process calls                                    │
│  ┌──── ae-shell-ui (Qt 6) ─────────────────────┐                     │
│  │  CapCut-style:                               │                     │
│  │    ▸ Library of effects (incl. third-party)  │                     │
│  │    ▸ Simplified track timeline               │                     │
│  │    ▸ Parameter inspector                     │                     │
│  │  Viewport = native AE Composition Viewer     │                     │
│  └──────────────────────────────────────────────┘                     │
└───────────────────────────────────────────────────────────────────────┘
```

The native AE Composition Viewer is reused for preview; the surrounding UI
is replaced by our Qt panel. Third-party paid effects work because AE itself
applies them — we just send `AEGP_ApplyEffect` calls.

See [docs/architecture.md](docs/architecture.md) for the threading model and
phase breakdown.

---

## Requirements

| Component | Version | Notes |
|---|---|---|
| Operating System | **Windows 10/11 (x64)** | macOS support is on the roadmap; not yet wired up. |
| Adobe After Effects | **2025** (24.x or 23.x also work) | Plugin is built against AE SDK 2025; backwards-compatible with AE 22+. |
| Adobe AE SDK | **2025** | Free download, requires Adobe ID. **Not** redistributable, never committed to this repo. |
| Visual Studio | **2022** (Community is fine) | Workload: *Desktop development with C++*. |
| Qt | **6.6+** (`win64_msvc2019_64` build is what Qt ships for 6.6/6.7; ABI-compatible with VS 2022) | LGPL build, free for this use. |
| CMake | **3.22+** | Bundled with VS 2022 is fine. |

---

## Setup

### 1. Install After Effects 2025

Via Adobe Creative Cloud Desktop:
<https://creativecloud.adobe.com/apps/download/creative-cloud>.

### 2. Download the AE SDK

1. Go to <https://developer.adobe.com/after-effects/>.
2. Click **Get the SDK** → sign in with Adobe ID.
3. Download `AfterEffectsSDK_Win_<date>.zip` for AE 2025.
4. Unzip to a stable path, e.g. `C:\dev\AdobeAfterEffectsSDK`.
5. Set the environment variable `ADOBE_AE_SDK` to that path:

   ```powershell
   [Environment]::SetEnvironmentVariable("ADOBE_AE_SDK", "C:\dev\AdobeAfterEffectsSDK", "User")
   ```

   Restart your shell (or VS) afterwards.

The SDK is **never** committed to this repository (Adobe NDA). Each developer
must download it locally.

### 3. Install Qt 6

1. Download the [Qt Online Installer](https://www.qt.io/download-open-source).
2. Pick the LGPL/Open Source option, free.
3. In the components selector pick **Qt 6.6+** → **MSVC 2019 64-bit** (this is Qt's official Windows toolchain for 6.6/6.7; the binaries are ABI-compatible with Visual Studio 2022). The `msvc2022_64` build only appears starting with Qt 6.8.
4. Note the install path, typically `C:\Qt\6.6.x\msvc2019_64`.
5. Set `CMAKE_PREFIX_PATH` to point there, or pass it on the CMake command line.

### 4. Clone

```powershell
git clone https://github.com/reideveloperr-jpg/ae-shell.git
cd ae-shell
```

---

## Build

### Option A — From the command line (CMake)

```powershell
cmake -S . -B build `
      -G "Visual Studio 17 2022" -A x64 `
      -DCMAKE_PREFIX_PATH="C:/Qt/6.6.3/msvc2019_64" `
      -DADOBE_AE_SDK="$env:ADOBE_AE_SDK"
cmake --build build --config Release
```

### Option B — From Visual Studio

Open the `ae-shell` folder in VS 2022 (it understands `CMakeLists.txt`
natively). Pick the `windows-x64-release` preset from
`CMakePresets.json` and click *Build*.

The output you care about is:

```
build/plugin/Release/ae-shell.aex            ← the plugin
build/ui/Release/ae-shell-ui-standalone.exe  ← Qt UI in standalone mode (for dev)
```

You can also build only the standalone UI without the AE SDK installed:

```powershell
cmake -S . -B build-ui-only -DAE_SHELL_BUILD_PLUGIN=OFF
cmake --build build-ui-only --config Release
```

This is what CI does on every push.

---

## Install the plugin in After Effects

1. Quit After Effects.
2. Copy `ae-shell.aex` to the AE plug-ins folder:

   ```
   C:\Program Files\Adobe\Adobe After Effects 2025\Support Files\Plug-ins\ae-shell\
   ```

   Create the `ae-shell` subfolder if it doesn't exist.
3. Launch After Effects.
4. Open menu **Window → AE Shell**. The Qt panel should appear.

If the menu item doesn't show up, check:
- `ae-shell.aex` is the **64-bit Release** build.
- The PiPL resource is embedded (verify with `dumpbin /HEADERS ae-shell.aex` —
  there should be a `PiPL` section, see [docs/architecture.md](docs/architecture.md)).
- AE version is 2022 or newer.

---

## Roadmap

| Phase | Goal | Status |
|---|---|:-:|
| **0** | Skeleton: AEGP plugin loads, registers menu command, opens empty Qt window | ✅ scaffolded |
| **1** | Enumerate all installed effects (incl. third-party) via `AEGP_EffectSuite` and show them in a searchable library panel | ⏳ |
| **2** | Apply selected effect to the active layer on double-click | ⏳ |
| **3** | Read active comp's layer/keyframe data → render a simplified custom timeline | ⏳ |
| **4** | Drag-and-drop effects onto clips; in/out trimming; speed | ⏳ |
| **5** | Parameter inspector with two-way keyframe sync | ⏳ |
| **6** | Templates / presets, media import, export through AE Render Queue | ⏳ |
| **7** | CapCut-style polish: dark theme, rounded cards, motion preview | ⏳ |

---

## License

MIT — see [LICENSE](LICENSE). Adobe trademarks belong to Adobe Inc.;
this project is **not** affiliated with Adobe. The AE SDK is **not** included.
Qt 6 is used under LGPL.
