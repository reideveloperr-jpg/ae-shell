#pragma once

class QApplication;

namespace ae_shell::ui {

// Loads the dark CapCut-style stylesheet from the compiled-in Qt resource
// and applies it process-wide.
void applyTheme(QApplication* app);

}  // namespace ae_shell::ui
