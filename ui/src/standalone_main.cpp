// standalone_main.cpp — entry point for `ae-shell-ui-standalone.exe`.
//
// This binary lets us iterate on the UI without needing After Effects.
// CI also builds it on every push to make sure the Qt code stays clean.

#include "UiHost.h"

int main(int /*argc*/, char* /*argv*/[]) {
    return ae_shell::ui::run_ui_event_loop();
}
