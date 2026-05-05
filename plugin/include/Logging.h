#pragma once

#include <string>
#include <string_view>

namespace ae_shell::log {

// Lightweight logger that writes to a file in `%TEMP%\ae-shell.log` so the
// user can `type` it after AE crashes without us having to bring in spdlog
// or fmt. Thread-safe.
void Init();
void Shutdown();

void Info (std::string_view msg);
void Warn (std::string_view msg);
void Error(std::string_view msg);

}  // namespace ae_shell::log
