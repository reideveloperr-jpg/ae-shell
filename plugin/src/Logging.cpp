// Logging.cpp — minimal file logger.
//
// In addition to writing each line to %TEMP%\ae-shell.log we also push the
// same line to OutputDebugStringA, so it shows up in DebugView / VS Output
// even if file I/O is misbehaving. Each write is fflush'd AND _commit'd
// (force-fsync to disk) so a subsequent process crash inside AE can't
// swallow the most recent diagnostics in the kernel write-back cache.

#include "Logging.h"

#include <chrono>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <mutex>
#include <string>

#if defined(_WIN32)
#   include <windows.h>
#   include <io.h>
#endif

namespace ae_shell::log {

namespace {

std::mutex   g_mutex;
std::FILE*   g_file = nullptr;

std::string LogPath() {
#if defined(_WIN32)
    char buf[MAX_PATH] = {};
    DWORD n = GetTempPathA(MAX_PATH, buf);
    if (n == 0 || n > MAX_PATH) {
        return "ae-shell.log";
    }
    return std::string(buf) + "ae-shell.log";
#else
    return "/tmp/ae-shell.log";
#endif
}

void WriteLine(const char* level, std::string_view msg) {
    std::lock_guard lk(g_mutex);

    // Always emit to the Windows debugger stream: this works even if
    // the file fopen failed (no %TEMP%, AE quarantined our process,
    // etc.) -- DebugView from Sysinternals captures everything.
    using namespace std::chrono;
    const auto now    = system_clock::now();
    const auto t      = system_clock::to_time_t(now);
    const auto ms     = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    std::tm    local{};
#if defined(_WIN32)
    localtime_s(&local, &t);
#else
    localtime_r(&t, &local);
#endif

    char line[1024] = {};
    std::snprintf(line, sizeof(line),
        "%04d-%02d-%02d %02d:%02d:%02d.%03lld [%s] %.*s\n",
        local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
        local.tm_hour, local.tm_min, local.tm_sec,
        static_cast<long long>(ms.count()),
        level,
        static_cast<int>(msg.size()), msg.data());

#if defined(_WIN32)
    OutputDebugStringA("[ae-shell] ");
    OutputDebugStringA(line);
#endif

    if (!g_file) return;

    std::fputs(line, g_file);
    std::fflush(g_file);
#if defined(_WIN32)
    // Force the kernel to push the write-back cache to disk so a crash
    // inside AE still leaves a readable log behind.
    int fd = _fileno(g_file);
    if (fd >= 0) {
        _commit(fd);
    }
#endif
}

}  // namespace

void Init() {
    std::lock_guard lk(g_mutex);
    if (g_file) return;

    // Open in truncate mode ('w') so each AE launch starts with a clean
    // log -- otherwise stale content from previous attempts confuses the
    // diagnosis. Disable stdio buffering so every fputs hits the OS
    // immediately and survives a hard crash.
    g_file = std::fopen(LogPath().c_str(), "w");
    if (g_file) {
        std::setvbuf(g_file, nullptr, _IONBF, 0);
    }
#if defined(_WIN32)
    OutputDebugStringA(g_file
        ? "[ae-shell] Logging::Init opened log file\n"
        : "[ae-shell] Logging::Init failed to open log file\n");
#endif
}

void Shutdown() {
    std::lock_guard lk(g_mutex);
    if (g_file) {
        std::fclose(g_file);
        g_file = nullptr;
    }
}

void Info (std::string_view msg) { WriteLine("INFO ", msg); }
void Warn (std::string_view msg) { WriteLine("WARN ", msg); }
void Error(std::string_view msg) { WriteLine("ERROR", msg); }

}  // namespace ae_shell::log
