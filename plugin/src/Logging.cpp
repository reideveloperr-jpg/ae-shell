// Logging.cpp — minimal file logger.

#include "Logging.h"

#include <chrono>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <mutex>
#include <string>

#if defined(_WIN32)
#   include <windows.h>
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
    if (!g_file) return;

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
    std::fprintf(g_file,
        "%04d-%02d-%02d %02d:%02d:%02d.%03lld [%s] %.*s\n",
        local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
        local.tm_hour, local.tm_min, local.tm_sec,
        static_cast<long long>(ms.count()),
        level,
        static_cast<int>(msg.size()), msg.data());
    std::fflush(g_file);
}

}  // namespace

void Init() {
    std::lock_guard lk(g_mutex);
    if (g_file) return;
    g_file = std::fopen(LogPath().c_str(), "a");
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
