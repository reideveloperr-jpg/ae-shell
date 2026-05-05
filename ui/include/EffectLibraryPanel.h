#pragma once

#include <QWidget>

class QListWidget;
class QLineEdit;

namespace ae_shell::ui {

// Phase 1 will populate this from `AEBridge::EnumerateInstalledEffects()`.
// For Phase 0 it shows a hard-coded preview list so the layout looks right.
class EffectLibraryPanel : public QWidget {
    Q_OBJECT
public:
    explicit EffectLibraryPanel(QWidget* parent = nullptr);

private:
    QLineEdit*   m_search = nullptr;
    QListWidget* m_list   = nullptr;
};

}  // namespace ae_shell::ui
